package com.mirceanealcos.SynthBridge.dto;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
public class MidiEventDto {

    @JsonProperty("note")
    private Integer note;
    @JsonProperty("timestamp")
    private Long timestamp;
    @JsonProperty("type")
    private String type;
    @JsonProperty("velocity")
    private Integer velocity;
    @JsonProperty("role")
    private String role;

    @Override
    public String toString() {
        return "MidiEventDto{" +
                "note=" + note +
                ", timestamp=" + timestamp +
                ", type='" + type + '\'' +
                ", velocity=" + velocity +
                '}';
    }
}
