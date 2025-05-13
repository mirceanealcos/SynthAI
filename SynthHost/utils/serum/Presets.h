//
// Created by Mircea Nealcos on 2/22/2025.
//

#ifndef PRESETS_H
#define PRESETS_H
#include <string>

class Preset {
public:

    Preset(const std::string& name, const std::string& path, const std::string& type) {
        this->name = name;
        this->path = path;
        this->type = type;
    }

    std::string name;
    std::string path;
    std::string type;
};

class Presets {
public:
    //basses
    static Preset ANALOG_REESE_SWEEP;
    static Preset VIBRATO_BASS;
    static Preset COLONY;
    static Preset ENGINE_HASH;
    static Preset NUMBERNINE;
    static Preset OFFRECORD;
    static Preset SUBNET;
    static Preset WELCOME;

    //leads
    static Preset LEAD_1984;
    static Preset CRASHWAVE;
    static Preset CURSED_BRASS;
    static Preset DS61;
    static Preset LEGATO_SAW_LEAD;
    static Preset MINI;
    static Preset MODULE;
    static Preset RETRO_BASS_LEAD;
    static Preset SAWKRAFT;
    static Preset TIMECOP;

    //pads
    static Preset BLADE_SWIMMER;
    static Preset BLESS;
    static Preset LALA;
    static Preset OUT_TO_PLAY;
    static Preset RETROTOOTH;
    static Preset SECONDS;
    static Preset VISIONS;

    //plucks
    static Preset RETROBIT;
    static Preset TETRA;

    static Preset getFromString(std::string preset);
};



#endif //PRESETS_H
