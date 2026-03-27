#include "bus.h"
#include <cstdint>

Bus::Bus() {
  // Clear RAM content
  for (auto &i : ram)
    i = 0x00;

  // Connect CPU to communicate with the Bus
  cpu.ConnectBus(this);
}

Bus::~Bus() {}

void Bus::write(uint16_t addr, uint8_t data) {
  if (addr >= 0x0000 && addr <= 0xFFF)
    ram[addr] = data;
}

uint8_t Bus::read(uint16_t addr, bool bReadOnly) {
  if (addr >= 0x0000 && addr <= 0xFFF)
    return ram[addr];

  return 0x0000;
}
