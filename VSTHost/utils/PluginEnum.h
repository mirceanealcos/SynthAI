//
// Created by Mircea Nealcos on 2/17/2025.
//
#ifndef PLUGINENUM_H
#define PLUGINENUM_H
#include <string>

class PluginDef {
public:
    PluginDef(const std::string &name, const std::string &path) {
        this->name = name;
        this->path = path;
    }
    std::string path;
    std::string name;
};

class PluginEnum {
public:
    static PluginDef SERUM;
};


#endif //PLUGINENUM_H
