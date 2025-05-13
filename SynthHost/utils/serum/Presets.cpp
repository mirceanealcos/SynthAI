//
// Created by Mircea Nealcos on 2/22/2025.
//

#include "Presets.h"

#include <stdexcept>
#include <unordered_map>

std::string presetDir = "D:/Projects/SynthAI/SynthHost/resources/presets/";

Preset Presets::ANALOG_REESE_SWEEP = Preset("Analog Reese Sweep",
                                            presetDir + "basses/analog_reese_sweep.vstpreset", "BASS");
Preset Presets::VIBRATO_BASS = Preset("Vibrato Bass",
                                      presetDir + "basses/analog_vibrato_bass.vstpreset", "BASS");
Preset Presets::COLONY = Preset("Colony",
                                presetDir + "basses/colony.vstpreset", "BASS");
Preset Presets::ENGINE_HASH = Preset("Engine Hash",
                                     presetDir + "basses/engine_hash.vstpreset", "BASS");
Preset Presets::NUMBERNINE = Preset("Numbernine",
                                    presetDir + "basses/numbernine.vstpreset", "BASS");
Preset Presets::OFFRECORD = Preset("Offrecord",
                                   presetDir + "basses/offrecord.vstpreset", "BASS");
Preset Presets::SUBNET = Preset("Subnet",
                                presetDir + "basses/subnet.vstpreset", "BASS");
Preset Presets::WELCOME = Preset("Welcome",
                                 presetDir + "basses/welcome.vstpreset", "BASS");
Preset Presets::LEAD_1984 = Preset("1984",
                                   presetDir + "leads/1984.vstpreset", "LEAD");
Preset Presets::CRASHWAVE = Preset("Crashwave",
                                   presetDir + "leads/crashwave.vstpreset", "LEAD");
Preset Presets::CURSED_BRASS = Preset("Cursed Brass",
                                      presetDir + "leads/cursed_brass.vstpreset", "LEAD");
Preset Presets::DS61 = Preset("DS61",
                              presetDir + "leads/ds61.vstpreset", "LEAD");
Preset Presets::LEGATO_SAW_LEAD = Preset("Legato Saw Lead",
                                         presetDir + "leads/legato_saw_lead.vstpreset", "LEAD");
Preset Presets::MINI = Preset("Mini",
                              presetDir + "leads/mini.vstpreset", "LEAD");
Preset Presets::MODULE = Preset("Module",
                                presetDir + "leads/module.vstpreset", "LEAD");
Preset Presets::RETRO_BASS_LEAD = Preset("Retro Bass Lead",
                                         presetDir + "leads/retro_bass_lead.vstpreset", "LEAD");
Preset Presets::SAWKRAFT = Preset("Sawkraft",
                                  presetDir + "leads/sawkraft.vstpreset", "LEAD");
Preset Presets::TIMECOP = Preset("Timecop",
                                 presetDir + "leads/timecop.vstpreset", "LEAD");
Preset Presets::BLADE_SWIMMER = Preset("Blade Swimmer",
                                       presetDir + "pads/blade_swimmer.vstpreset", "PAD");
Preset Presets::BLESS = Preset("Bless",
                               presetDir + "pads/bless.vstpreset", "PAD");
Preset Presets::LALA = Preset("Lala",
                              presetDir + "pads/lala.vstpreset", "PAD");
Preset Presets::OUT_TO_PLAY = Preset("Out To Play",
                                     presetDir + "pads/out_to_play.vstpreset", "PAD");
Preset Presets::RETROTOOTH = Preset("Retrotooth",
                                    presetDir + "pads/retrotooth.vstpreset", "PAD");
Preset Presets::SECONDS = Preset("Seconds",
                                 presetDir + "pads/seconds.vstpreset", "PAD");
Preset Presets::VISIONS = Preset("Visions",
                                 presetDir + "pads/visions.vstpreset", "PAD");
Preset Presets::RETROBIT = Preset("Retrobit",
                                  presetDir + "plucks/retrobit.vstpreset", "PLUCK");
Preset Presets::TETRA = Preset("Tetra",
                               presetDir + "plucks/tetra.vstpreset", "PLUCK");

Preset Presets::getFromString(std::string name) {
    static const std::unordered_map<std::string, Preset> map = {
        {"ANALOG_REESE_SWEEP", Presets::ANALOG_REESE_SWEEP},
        {"VIBRATO_BASS", Presets::VIBRATO_BASS},
        {"COLONY", Presets::COLONY},
        {"ENGINE_HASH", Presets::ENGINE_HASH},
        {"NUMBERNINE", Presets::NUMBERNINE},
        {"OFFRECORD", Presets::OFFRECORD},
        {"SUBNET", Presets::SUBNET},
        {"WELCOME", Presets::WELCOME},
        {"LEAD_1984", Presets::LEAD_1984},
        {"CRASHWAVE", Presets::CRASHWAVE},
        {"CURSED_BRASS", Presets::CURSED_BRASS},
        {"DS61", Presets::DS61},
        {"LEGATO_SAW_LEAD", Presets::LEGATO_SAW_LEAD},
        {"MINI", Presets::MINI},
        {"MODULE", Presets::MODULE},
        {"RETRO_BASS_LEAD", Presets::RETRO_BASS_LEAD},
        {"SAWKRAFT", Presets::SAWKRAFT},
        {"TIMECOP", Presets::TIMECOP},
        {"BLADE_SWIMMER", Presets::BLADE_SWIMMER},
        {"BLESS", Presets::BLESS},
        {"LALA", Presets::LALA},
        {"OUT_TO_PLAY", Presets::OUT_TO_PLAY},
        {"RETROTOOTH", Presets::RETROTOOTH},
        {"SECONDS", Presets::SECONDS},
        {"VISIONS", Presets::VISIONS},
        {"RETROBIT", Presets::RETROBIT},
        {"TETRA", Presets::TETRA}
    };

    auto it = map.find(name);
    if (it != map.end())
        return it->second;

    throw std::invalid_argument("Unknown preset name: " + name);
}
