#pragma once
#include "Cartridge.h"
#include "olc2C02.h"
#include "olc6502.h"
#include <array>
#include <cstdint>
#include <memory>

class Bus {
public:
  Bus();
  ~Bus();

public: // Devices on Bus
  // The 6502 CPU
  olc6502 cpu;

  // The 2C02 Picture Processing Unit
  olc2C02 ppu;

  // Fake 64 byte RAM for now
  std::array<uint8_t, 2048> cpuRam;

  // The Cartridge
  std::shared_ptr<Cartridge> cart;

public: // Bus Read & Write
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

public: // Stystem Interface
  void insertCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void reset();
  void clock();

private: // Count how many clocks have tciked
  uint32_t nSystemClockCounter = 0;
};
