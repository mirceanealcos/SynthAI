//
// Created by Mircea Nealcos on 2/22/2025.
//

#include "Presets.h"

#include <stdexcept>
#include <unordered_map>

//std::string presetDir = "C:/Projects/SynthHost/resources/presets/";
std::string presetDir = "D:/Projects/SynthAI/SynthHost/resources/presets/";

Preset Presets::ANALOG_REESE_SWEEP = Preset("Analog Reese Sweep",
                                            presetDir + "basses/analog_reese_sweep.vstpreset", "bass");
Preset Presets::VIBRATO_BASS = Preset("Vibrato Bass",
                                      presetDir + "basses/analog_vibrato_bass.vstpreset", "bass");
Preset Presets::COLONY = Preset("Colony",
                                presetDir + "basses/colony.vstpreset", "bass");
Preset Presets::ENGINE_HASH = Preset("Engine Hash",
                                     presetDir + "basses/engine_hash.vstpreset", "bass");
Preset Presets::NUMBERNINE = Preset("Numbernine",
                                    presetDir + "basses/numbernine.vstpreset", "bass");
Preset Presets::OFFRECORD = Preset("Offrecord",
                                   presetDir + "basses/offrecord.vstpreset", "bass");
Preset Presets::SUBNET = Preset("Subnet",
                                presetDir + "basses/subnet.vstpreset", "bass");
Preset Presets::WELCOME = Preset("Welcome",
                                 presetDir + "basses/welcome.vstpreset", "bass");
Preset Presets::LEAD_1984 = Preset("1984",
                                   presetDir + "leads/1984.vstpreset", "lead");
Preset Presets::CRASHWAVE = Preset("Crashwave",
                                   presetDir + "leads/crashwave.vstpreset", "lead");
Preset Presets::CURSED_BRASS = Preset("Cursed Brass",
                                      presetDir + "leads/cursed_brass.vstpreset", "lead");
Preset Presets::DS61 = Preset("DS61",
                              presetDir + "leads/ds61.vstpreset", "lead");
Preset Presets::LEGATO_SAW_LEAD = Preset("Legato Saw Lead",
                                         presetDir + "leads/legato_saw_lead.vstpreset", "lead");
Preset Presets::MINI = Preset("Mini",
                              presetDir + "leads/mini.vstpreset", "lead");
Preset Presets::MODULE = Preset("Module",
                                presetDir + "leads/module.vstpreset", "lead");
Preset Presets::RETRO_BASS_LEAD = Preset("Retro Bass Lead",
                                         presetDir + "leads/retro_bass_lead.vstpreset", "lead");
Preset Presets::SAWKRAFT = Preset("Sawkraft",
                                  presetDir + "leads/sawkraft.vstpreset", "lead");
Preset Presets::TIMECOP = Preset("Timecop",
                                 presetDir + "leads/timecop.vstpreset", "lead");
Preset Presets::BLADE_SWIMMER = Preset("Blade Swimmer",
                                       presetDir + "pads/blade_swimmer.vstpreset", "pad");
Preset Presets::BLESS = Preset("Bless",
                               presetDir + "pads/bless.vstpreset", "pad");
Preset Presets::LALA = Preset("Lala",
                              presetDir + "pads/lala.vstpreset", "pad");
Preset Presets::OUT_TO_PLAY = Preset("Out To Play",
                                     presetDir + "pads/out_to_play.vstpreset", "pad");
Preset Presets::RETROTOOTH = Preset("Retrotooth",
                                    presetDir + "pads/retrotooth.vstpreset", "pad");
Preset Presets::SECONDS = Preset("Seconds",
                                 presetDir + "pads/seconds.vstpreset", "pad");
Preset Presets::VISIONS = Preset("Visions",
                                 presetDir + "pads/visions.vstpreset", "pad");
Preset Presets::RETROBIT = Preset("Retrobit",
                                  presetDir + "plucks/retrobit.vstpreset", "pluck");
Preset Presets::TETRA = Preset("Tetra",
                               presetDir + "plucks/tetra.vstpreset", "pluck");

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
