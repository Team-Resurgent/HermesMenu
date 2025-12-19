#pragma once

#include "Integers.h"

#include <string>
#include <vector>

class Menu
{
public:
	Menu();
    ~Menu();
    bool LoadTrainer(std::string mountPath);
	void Update();
	void Render();
private:
    std::string mountPath_;
	int32_t selectedControl_;
    uint32_t trainerSize_;
    uint8_t* trainerData_;
    std::vector<std::string> trainerOptions_;

};
