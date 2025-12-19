#pragma once

#include "Integers.h"

#include <string>
#include <vector>

class Trainer
{
public:
    static uint8_t* LoadTrainer(std::string path, uint32_t* trainerSize);
    static std::vector<std::string> GetOptions(uint8_t* trainerData, uint32_t trainerSize);
    static void ListTitleids(uint8_t* trainerData, uint32_t trainerSize);
    static bool GetOption(uint8_t* trainerData, uint32_t trainerSize, uint32_t index);
    static void ToggleOption(uint8_t* trainerData, uint32_t trainerSize, uint32_t index);
};
