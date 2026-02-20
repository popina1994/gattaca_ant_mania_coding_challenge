#include "utils.h"
#pragma once

struct Ant
{
    int id;
    std::string colonyName;
    bool alive;

    static int glId;

    Ant(): id(glId++){}
};

int Ant::glId = 0;