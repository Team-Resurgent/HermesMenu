#pragma once

#include "External.h"
#include "utils.h"

class audioPlayer {
  public:
    static bool init();
  private:
    static uint64_t WINAPI process(void* param);
};
