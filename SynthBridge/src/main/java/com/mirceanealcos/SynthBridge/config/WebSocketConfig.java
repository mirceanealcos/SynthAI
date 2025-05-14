package com.mirceanealcos.SynthBridge.config;

import com.mirceanealcos.SynthBridge.dto.MidiEventDto;
import com.mirceanealcos.SynthBridge.dto.PresetChangeDto;
import com.mirceanealcos.SynthBridge.handler.JsonWebSocketHandler;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

@Configuration
@EnableWebSocket
public class WebSocketConfig implements WebSocketConfigurer {

    @Override
    public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
        registry.addHandler(new JsonWebSocketHandler<>(PresetChangeDto.class), "/user/preset")
                .setAllowedOrigins("*")
                .addHandler(new JsonWebSocketHandler<>(MidiEventDto.class), "/user/input")
                .setAllowedOrigins("*");
    }

}
