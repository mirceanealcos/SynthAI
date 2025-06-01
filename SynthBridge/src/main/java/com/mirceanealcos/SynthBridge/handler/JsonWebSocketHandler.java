package com.mirceanealcos.SynthBridge.handler;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import io.github.bucket4j.Bandwidth;
import io.github.bucket4j.Bucket;
import io.micrometer.core.instrument.Counter;
import io.micrometer.core.instrument.Gauge;
import io.micrometer.core.instrument.MeterRegistry;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicInteger;

public class JsonWebSocketHandler<T> extends TextWebSocketHandler {

    private static final Logger log = LoggerFactory.getLogger(JsonWebSocketHandler.class);
    private static final Integer MAX_CONNECTION_COUNT = 10;
    private static final Integer MAX_RECONNECTION_ATTEMPTS = 10;
    private static final Integer MAX_ERROR_COUNT = 10;
    private static final Bandwidth MESSAGE_LIMIT = Bandwidth.builder()
            .capacity(100)
            .refillIntervally(100, Duration.ofSeconds(1))
            .build();

    private final Map<String, AtomicInteger> reconnectionAttempts = Collections.synchronizedMap(new HashMap<>());
    private final ConcurrentMap<WebSocketSession, Bucket> buckets = new ConcurrentHashMap<>();
    private final Set<WebSocketSession> sessions = Collections.synchronizedSet(new HashSet<>());
    private final ObjectMapper mapper = new ObjectMapper();
    private final Class<T> payloadType;
    private final String handlerName;

    private final Counter messageCounter;
    private final Counter errorCounter;
    private final Counter disconnectCounter;
    private final Gauge activeSessionsGauge;

    public JsonWebSocketHandler(Class<T> payloadType, MeterRegistry meterRegistry, String handlerName) {
        this.payloadType = payloadType;
        this.handlerName = handlerName;
        this.messageCounter = meterRegistry.counter("total_messages", "handler", handlerName);
        this.errorCounter = meterRegistry.counter("total_errors", "handler", handlerName);
        this.disconnectCounter = meterRegistry.counter("total_disconnects", "handler", handlerName);
        this.activeSessionsGauge = Gauge.builder("active_connections", sessions, Set::size)
                .description("Currently active WebSocket connections")
                .tag("handler", handlerName)
                .register(meterRegistry);
    }

    @Override
    public void afterConnectionEstablished(WebSocketSession session) throws IOException {
        InetSocketAddress peerAddress = session.getRemoteAddress();
        String peerIP = (peerAddress == null) ? "unknown" : peerAddress.getAddress().getHostAddress();
        if (reconnectionAttempts.containsKey(peerIP)) {
            AtomicInteger reconnectionCount = reconnectionAttempts.get(peerIP);
            int count = reconnectionCount.incrementAndGet();
            if (count > MAX_RECONNECTION_ATTEMPTS) {
                log.warn("Blocking connection to " + peerIP + ", too many reconnection attempts.");
                session.close(CloseStatus.POLICY_VIOLATION);
                return;
            }
        }
        else {
            reconnectionAttempts.put(peerIP, new AtomicInteger(1));
        }

        synchronized (sessions) {
            if (sessions.size() >= MAX_CONNECTION_COUNT) {
                log.warn("Session limit reached on handler " + handlerName + ", ignoring connection to " + session.getId() + ".");
                session.close(CloseStatus.POLICY_VIOLATION);
                return;
            }
        }
        Bucket bucket = Bucket.builder()
                .addLimit(MESSAGE_LIMIT)
                .build();
        buckets.put(session, bucket);
        sessions.add(session);
        log.info("Successfully established connection to " + session.getId());
    }

    @Override
    protected void handleTextMessage(WebSocketSession session, TextMessage message) throws Exception {
        synchronized (sessions) {
            if (!sessions.contains(session)) {
                session.close();
                return;
            }
        }
        Bucket bucket = buckets.get(session);
        if (bucket.tryConsume(1)) {
            messageCounter.increment();
            try {
                T json = mapper.readValue(message.getPayload(), payloadType);
                synchronized (sessions) {
                    for (WebSocketSession s : sessions) {
                        if (s.isOpen() && !session.equals(s)) {
                            s.sendMessage(new TextMessage(mapper.writeValueAsString(json)));
                        }
                    }
                }
            } catch (JsonProcessingException e) {
                log.error(e.getMessage(), e);
                errorCounter.increment();
                session.close(CloseStatus.SERVER_ERROR);
                sessions.remove(session);
                buckets.remove(session);
            }
        }
        else {
            log.warn("Shutting down session " + session.getId() + ". (exceeded " + MESSAGE_LIMIT.getCapacity() + " messages/sec)");
            session.close(CloseStatus.POLICY_VIOLATION);
        }
    }

    @Override
    public void handleTransportError(WebSocketSession session, Throwable exception) throws Exception {
        sessions.remove(session);
        buckets.remove(session);
        session.close(CloseStatus.SERVER_ERROR);
        log.error("Transport error: " + exception.getMessage());
        errorCounter.increment();
    }

    @Override
    public void afterConnectionClosed(WebSocketSession session, CloseStatus status) {
        log.info("Connection to " + session.getId() + " closed.");
        sessions.remove(session);
        buckets.remove(session);
        disconnectCounter.increment();
    }
}
