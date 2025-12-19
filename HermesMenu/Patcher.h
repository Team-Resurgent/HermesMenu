#pragma once

#include "Integers.h"

class Patcher
{
public:
    static bool InstallTrainer(uint8_t* trainerData, uint32_t trainerSize);
    static bool RemoveTrainer();
};
