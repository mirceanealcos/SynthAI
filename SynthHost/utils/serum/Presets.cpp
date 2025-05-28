//
// Created by Mircea Nealcos on 2/22/2025.
//

#include "Presets.h"

#include <random>
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

Preset Presets::getRandomBass() {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 8);
    int presetIndex = distr(gen);
    if (presetIndex == 1)
        return ANALOG_REESE_SWEEP;
    if (presetIndex == 2)
        return VIBRATO_BASS;
    if (presetIndex == 3)
        return COLONY;
    if (presetIndex == 4)
        return ENGINE_HASH;
    if (presetIndex == 5)
        return NUMBERNINE;
    if (presetIndex == 6)
        return OFFRECORD;
    if (presetIndex == 7)
        return SUBNET;
    if (presetIndex == 8)
        return WELCOME;
    return ANALOG_REESE_SWEEP;
}

Preset Presets::getRandomLead() {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 10);
    int presetIndex = distr(gen);
    if (presetIndex == 1)
        return LEAD_1984;
    if (presetIndex == 2)
        return CRASHWAVE;
    if (presetIndex == 3)
        return CURSED_BRASS;
    if (presetIndex == 4)
        return DS61;
    if (presetIndex == 5)
        return LEGATO_SAW_LEAD;
    if (presetIndex == 6)
        return MINI;
    if (presetIndex == 7)
        return MODULE;
    if (presetIndex == 8)
        return RETRO_BASS_LEAD;
    if (presetIndex == 9)
        return SAWKRAFT;
    if (presetIndex == 10)
        return TIMECOP;
    return LEAD_1984;
}

Preset Presets::getRandomPad() {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 7);
    int presetIndex = distr(gen);
    if (presetIndex == 1)
        return BLADE_SWIMMER;
    if (presetIndex == 2)
        return BLESS;
    if (presetIndex == 3)
        return LALA;
    if (presetIndex == 4)
        return OUT_TO_PLAY;
    if (presetIndex == 5)
        return RETROTOOTH;
    if (presetIndex == 6)
        return SECONDS;
    if (presetIndex == 7)
        return VISIONS;
    return BLADE_SWIMMER;
}

Preset Presets::getRandomPluck() {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 7);
    int presetIndex = distr(gen);
    if (presetIndex == 1)
        return RETROBIT;
    if (presetIndex == 2)
        return TETRA;
    return RETROBIT;
}





