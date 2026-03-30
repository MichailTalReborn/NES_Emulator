#pragma once

#include "Cartridge.h"
#include <cstdint>
#include <memory>
class olc2C02 {
public:
  olc2C02();
  ~olc2C02();

  // Communicate with main bus
  uint8_t cpuRead(uint16_t addr, bool rdonly = false);
  void cpuWrite(uint16_t addr, uint8_t data);

  // Communicate with PPU bus
  uint8_t ppuRead(uint16_t addr, bool rdonly = false);
  void ppuWrite(uint16_t addr, uint8_t data);

private:
  uint8_t tblName[2][1024];    // Table Name
  uint8_t tblPalette[32];      // Pallettes^
  uint8_t tblPattern[2][4096]; // Unused, should be on Cartridge

private: // The Cartridge
  std::shared_ptr<Cartridge> cart;

public: // Interfacte
  void ConnectCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void clock();

private:
  int16_t scanline = 0;
  int16_t cycle = 0;

public:
  bool frame_complete = false;
};
