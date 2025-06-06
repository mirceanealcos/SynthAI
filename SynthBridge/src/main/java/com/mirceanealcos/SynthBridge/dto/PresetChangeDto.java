package com.mirceanealcos.SynthBridge.dto;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.mirceanealcos.SynthBridge.dto.enums.Preset;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
public class PresetChangeDto {

    @JsonProperty("preset")
    private Preset preset;

}
