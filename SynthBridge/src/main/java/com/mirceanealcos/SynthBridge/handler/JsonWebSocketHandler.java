package com.mirceanealcos.SynthBridge.handler;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.mirceanealcos.SynthBridge.dto.ErrorResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public class JsonWebSocketHandler<T> extends TextWebSocketHandler {

    private static final Logger log = LoggerFactory.getLogger(JsonWebSocketHandler.class);
    private final Set<WebSocketSession> sessions = Collections.synchronizedSet(new HashSet<>());
    private final ObjectMapper mapper = new ObjectMapper();
    private final Class<T> payloadType;

    public JsonWebSocketHandler(Class<T> payloadType) {
        this.payloadType = payloadType;
    }

    @Override
    public void afterConnectionEstablished(WebSocketSession session) {
        log.info("Successfully established connection to " + session.getId());
        sessions.add(session);
    }

    @Override
    protected void handleTextMessage(WebSocketSession session, TextMessage message) throws Exception {
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
            ErrorResponse errorResponse = new ErrorResponse("invalid payload");
            session.sendMessage(new TextMessage(mapper.writeValueAsString(errorResponse)));
        }
    }

    @Override
    public void handleTransportError(WebSocketSession session, Throwable exception) throws Exception {
        sessions.remove(session);
        session.close(CloseStatus.SERVER_ERROR);
        log.error("Transport error: " + exception.getMessage());
    }

    @Override
    public void afterConnectionClosed(WebSocketSession session, CloseStatus status) {
        log.info("Connection to " + session.getId() + " closed.");
        sessions.remove(session);
    }
}
