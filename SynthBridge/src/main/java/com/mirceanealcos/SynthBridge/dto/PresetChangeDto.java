package com.mirceanealcos.SynthBridge.dto;

import com.mirceanealcos.SynthBridge.dto.enums.Preset;
import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class PresetChangeDto {

    private Preset preset;

}
