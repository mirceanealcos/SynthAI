package com.mirceanealcos.SynthBridge.config;

import com.mirceanealcos.SynthBridge.dto.MidiEventDto;
import com.mirceanealcos.SynthBridge.dto.PresetChangeDto;
import com.mirceanealcos.SynthBridge.handler.JsonWebSocketHandler;
import io.micrometer.core.instrument.MeterRegistry;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

@Configuration
@EnableWebSocket
public class WebSocketConfig implements WebSocketConfigurer {

    private final MeterRegistry meterRegistry;

    @Autowired
    public WebSocketConfig(MeterRegistry meterRegistry) {
        this.meterRegistry = meterRegistry;
    }

    @Override
    public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
        registry.addHandler(new JsonWebSocketHandler<>(PresetChangeDto.class, meterRegistry, "preset_handler"), "/user/preset")
                .addHandler(new JsonWebSocketHandler<>(MidiEventDto.class, meterRegistry,  "user_midi_input_handler"), "/user/input")
                .addHandler(new JsonWebSocketHandler<>(MidiEventDto.class, meterRegistry,  "ai_midi_output_handler"), "/composer/output")
                .setAllowedOrigins("*");
    }

}
