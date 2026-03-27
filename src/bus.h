#pragma once
#include "olc6502.h"
#include <array>
#include <cstdint>

class Bus {
public:
  Bus();
  ~Bus();

public: // Devices on Bus
  olc6502 cpu;

  // Fake 64 byte RAM for now
  std::array<uint8_t, 64 * 1024> ram;

public: // Bus Read & Write
  void write(uint16_t addr, uint8_t data);
  uint8_t read(uint16_t addr, bool bReadOnly = false);
};
