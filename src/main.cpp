
#include <deque>
#include <iostream>
#include <sstream>
#include <string>

#include "Bus.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

// ============================================================
//  CLI flags parsed in main() and passed into the app
// ============================================================
struct AppConfig {
  std::string sRomPath;
  bool bShowAudio = false;
  bool bShowCode = false;
  bool bShowRam = false;
  bool bShowCpu = false;
  bool bDebugMode = false; // true if ANY debug panel is active
};

class NES_Emulator : public olc::PixelGameEngine {
public:
  NES_Emulator(const AppConfig &cfg) : config(cfg) {
    sAppName = "NES_Emulator";
  }

private:
  AppConfig config;

  // The NES
  Bus nes;
  std::shared_ptr<Cartridge> cart;
  bool bEmulationRun = false;
  float fResidualTime = 0.0f;

  uint8_t nSelectedPalette = 0x00;

  std::list<uint16_t> audio[4];
  float fAccumulatedTime = 0.0f;

  static NES_Emulator *pInstance;

  static float SoundOut(int nChannel, float fGlobalTime, float fTimeStep) {
    if (nChannel == 0) {
      while (!pInstance->nes.clock()) {
      }
      return static_cast<float>(pInstance->nes.dAudioSample);
    }
    return 0.0f;
  }

private:
  // ── Utility ────────────────────────────────────────────────
  std::map<uint16_t, std::string> mapAsm;

  std::string hex(uint32_t n, uint8_t d) {
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
      s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
  }

  // ── Debug draw helpers ──────────────────────────────────────
  void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns) {
    int nRamX = x, nRamY = y;
    for (int row = 0; row < nRows; row++) {
      std::string sOffset = "$" + hex(nAddr, 4) + ":";
      for (int col = 0; col < nColumns; col++) {
        sOffset += " " + hex(nes.cpuRead(nAddr, true), 2);
        nAddr += 1;
      }
      DrawString(nRamX, nRamY, sOffset);
      nRamY += 10;
    }
  }

  void DrawCpu(int x, int y) {
    DrawString(x, y, "STATUS:", olc::WHITE);
    DrawString(x + 64, y, "N",
               nes.cpu.status & olc6502::N ? olc::GREEN : olc::RED);
    DrawString(x + 80, y, "V",
               nes.cpu.status & olc6502::V ? olc::GREEN : olc::RED);
    DrawString(x + 96, y, "-",
               nes.cpu.status & olc6502::U ? olc::GREEN : olc::RED);
    DrawString(x + 112, y, "B",
               nes.cpu.status & olc6502::B ? olc::GREEN : olc::RED);
    DrawString(x + 128, y, "D",
               nes.cpu.status & olc6502::D ? olc::GREEN : olc::RED);
    DrawString(x + 144, y, "I",
               nes.cpu.status & olc6502::I ? olc::GREEN : olc::RED);
    DrawString(x + 160, y, "Z",
               nes.cpu.status & olc6502::Z ? olc::GREEN : olc::RED);
    DrawString(x + 178, y, "C",
               nes.cpu.status & olc6502::C ? olc::GREEN : olc::RED);
    DrawString(x, y + 10, "PC: $" + hex(nes.cpu.pc, 4));
    DrawString(x, y + 20,
               "A:  $" + hex(nes.cpu.a, 2) + "  [" + std::to_string(nes.cpu.a) +
                   "]");
    DrawString(x, y + 30,
               "X:  $" + hex(nes.cpu.x, 2) + "  [" + std::to_string(nes.cpu.x) +
                   "]");
    DrawString(x, y + 40,
               "Y:  $" + hex(nes.cpu.y, 2) + "  [" + std::to_string(nes.cpu.y) +
                   "]");
    DrawString(x, y + 50, "Stack P: $" + hex(nes.cpu.stkp, 4));
  }

  void DrawCode(int x, int y, int nLines) {
    auto it_a = mapAsm.find(nes.cpu.pc);
    int nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      DrawString(x, nLineY, (*it_a).second, olc::CYAN);
      while (nLineY < (nLines * 10) + y) {
        nLineY += 10;
        if (++it_a != mapAsm.end())
          DrawString(x, nLineY, (*it_a).second);
      }
    }
    it_a = mapAsm.find(nes.cpu.pc);
    nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      while (nLineY > y) {
        nLineY -= 10;
        if (--it_a != mapAsm.end())
          DrawString(x, nLineY, (*it_a).second);
      }
    }
  }

  void DrawAudio(int channel, int x, int y) {
    FillRect(x, y, 120, 120, olc::BLACK);
    int i = 0;
    for (auto s : audio[channel]) {
      Draw(x + i, y + (s >> (channel == 2 ? 5 : 4)), olc::YELLOW);
      i++;
    }
  }

  // ── Lifecycle ───────────────────────────────────────────────
  bool OnUserCreate() override {
    cart = std::make_shared<Cartridge>(config.sRomPath);

    if (!cart->ImageValid()) {
      std::cerr << "NES_Emulator: failed to load ROM: " << config.sRomPath
                << "\n";
      return false;
    }

    nes.insertCartridge(cart);

    if (config.bShowCode)
      mapAsm = nes.cpu.disassemble(0x0000, 0xFFFF);

    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 120; j++)
        audio[i].push_back(0);

    nes.reset();

    pInstance = this;
    nes.SetSampleFrequency(44100);
    olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
    olc::SOUND::SetUserSynthFunction(SoundOut);
    return true;
  }

  bool OnUserDestroy() override {
    olc::SOUND::DestroyAudio();
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    // ── Shared controller input ─────────────────────────────
    nes.controller[0] = 0x00;
    nes.controller[0] |= GetKey(olc::Key::X).bHeld ? 0x80 : 0x00; // A
    nes.controller[0] |= GetKey(olc::Key::Z).bHeld ? 0x40 : 0x00; // B
    nes.controller[0] |= GetKey(olc::Key::A).bHeld ? 0x20 : 0x00; // Select
    nes.controller[0] |= GetKey(olc::Key::S).bHeld ? 0x10 : 0x00; // Start
    nes.controller[0] |= GetKey(olc::Key::UP).bHeld ? 0x08 : 0x00;
    nes.controller[0] |= GetKey(olc::Key::DOWN).bHeld ? 0x04 : 0x00;
    nes.controller[0] |= GetKey(olc::Key::LEFT).bHeld ? 0x02 : 0x00;
    nes.controller[0] |= GetKey(olc::Key::RIGHT).bHeld ? 0x01 : 0x00;

    if (GetKey(olc::Key::R).bPressed)
      nes.reset();
    if (GetKey(olc::Key::P).bPressed)
      (++nSelectedPalette) &= 0x07;

    // SPACE toggles pause only when not audio-synced (debug mode)
    if (config.bDebugMode && GetKey(olc::Key::SPACE).bPressed)
      bEmulationRun = !bEmulationRun;

    Clear(olc::DARK_BLUE);

    // ── Emulation tick ──────────────────────────────────────
    if (!config.bDebugMode) {
      // Audio-synced: sound thread drives emulation via SoundOut()
      fAccumulatedTime += fElapsedTime;
      if (fAccumulatedTime >= 1.0f / 60.0f) {
        fAccumulatedTime -= (1.0f / 60.0f);
        if (config.bShowAudio) {
          audio[0].pop_front();
          audio[0].push_back(nes.apu.pulse1_visual);
          audio[1].pop_front();
          audio[1].push_back(nes.apu.pulse2_visual);
          audio[2].pop_front();
          audio[2].push_back(nes.apu.noise_visual);
        }
      }
    } else {
      // Debug mode: manual stepping
      if (bEmulationRun) {
        if (fResidualTime > 0.0f)
          fResidualTime -= fElapsedTime;
        else {
          fResidualTime += (1.0f / 60.0f) - fElapsedTime;
          do {
            nes.clock();
          } while (!nes.ppu.frame_complete);
          nes.ppu.frame_complete = false;
        }
      } else {
        // C = single CPU instruction
        if (GetKey(olc::Key::C).bPressed) {
          do {
            nes.clock();
          } while (!nes.cpu.complete());
          do {
            nes.clock();
          } while (nes.cpu.complete());
        }
        // F = single frame
        if (GetKey(olc::Key::F).bPressed) {
          do {
            nes.clock();
          } while (!nes.ppu.frame_complete);
          do {
            nes.clock();
          } while (!nes.cpu.complete());
          nes.ppu.frame_complete = false;
        }
      }
    }

    // ── Draw game screen ────────────────────────────────────
    // Full-window mode: scale x2 fills 512x480
    // Debug mode: game sits at left, panels on the right from x=516
    int nGameScale = config.bDebugMode ? 2 : 2;
    DrawSprite(0, 0, &nes.ppu.GetScreen(), nGameScale);

    // ── Debug panels (only when their flag is set) ──────────
    if (config.bDebugMode) {
      constexpr int PANEL_X = 516;

      if (config.bShowCpu) {
        DrawCpu(PANEL_X, 2);
      }

      if (config.bShowCode) {
        DrawCode(PANEL_X, config.bShowCpu ? 72 : 2, 26);
      }

      if (config.bShowRam) {
        int ramY = 2;
        if (config.bShowCpu)
          ramY += 70;
        if (config.bShowCode)
          ramY += 270;
        DrawRam(PANEL_X, ramY, 0x0000, 16, 16);
        DrawRam(PANEL_X, ramY + 182, 0x8000, 16, 16);
      }

      if (config.bShowAudio) {
        DrawAudio(0, 520, 72);
        DrawAudio(1, 644, 72);
        DrawAudio(2, 520, 196);
        DrawAudio(3, 644, 196);
      }

      // Palettes & pattern tables always shown in debug mode
      constexpr int nSwatchSize = 6;
      for (int p = 0; p < 8; p++)
        for (int s = 0; s < 4; s++)
          FillRect(PANEL_X + p * (nSwatchSize * 5) + s * nSwatchSize, 340,
                   nSwatchSize, nSwatchSize,
                   nes.ppu.GetColourFromPaletteRam(p, s));

      DrawRect(PANEL_X + nSelectedPalette * (nSwatchSize * 5) - 1, 339,
               (nSwatchSize * 4), nSwatchSize, olc::WHITE);

      DrawSprite(PANEL_X, 348, &nes.ppu.GetPatternTable(0, nSelectedPalette));
      DrawSprite(PANEL_X + 132, 348,
                 &nes.ppu.GetPatternTable(1, nSelectedPalette));
    }

    return true;
  }
};

NES_Emulator *NES_Emulator::pInstance = nullptr;

// ============================================================
//  Help
// ============================================================
static void printHelp(const char *argv0) {
  std::cout
      << "Usage: " << argv0
      << " <rom.nes> [options]\n"
         "\n"
         "Options:\n"
         "  -h, --help      Show this help message and exit\n"
         "\n"
         "Debug panels (each adds a side panel; without any the game runs "
         "full-window):\n"
         "  --cpu           Show CPU register status\n"
         "  --code          Show disassembled code around the program counter\n"
         "  --ram           Show zero-page and PRG RAM hex dumps\n"
         "  --audio         Show APU channel waveform visualisers\n"
         "\n"
         "Debug controls (only active when at least one panel is enabled):\n"
         "  SPACE           Pause / resume emulation\n"
         "  C               Step one CPU instruction (while paused)\n"
         "  F               Step one full frame (while paused)\n"
         "\n"
         "Controls (always active):\n"
         "  Arrow keys      D-Pad\n"
         "  X               A button\n"
         "  Z               B button\n"
         "  A               Select\n"
         "  S               Start\n"
         "  R               Reset\n"
         "  P               Cycle palette\n";
}

// ============================================================
//  Entry point
// ============================================================
int main(int argc, char *argv[]) {
  AppConfig cfg;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printHelp(argv[0]);
      return 0;
    } else if (arg == "--cpu") {
      cfg.bShowCpu = true;
    } else if (arg == "--code") {
      cfg.bShowCode = true;
    } else if (arg == "--ram") {
      cfg.bShowRam = true;
    } else if (arg == "--audio") {
      cfg.bShowAudio = true;
    } else if (arg[0] != '-' && cfg.sRomPath.empty()) {
      cfg.sRomPath = arg;
    } else {
      std::cerr << "NES_Emulator: unknown option '" << arg << "'\n";
      std::cerr << "Run with --help for usage.\n";
      return 1;
    }
  }

  if (cfg.sRomPath.empty()) {
    std::cerr << "NES_Emulator: no ROM specified.\n";
    std::cerr << "Run with --help for usage.\n";
    return 1;
  }

  cfg.bDebugMode =
      cfg.bShowCpu || cfg.bShowCode || cfg.bShowRam || cfg.bShowAudio;

  // Window size: full game only = 512x480, debug panel = 780x480
  const int nWidth = cfg.bDebugMode ? 780 : 512;
  const int nHeight = 480;

  NES_Emulator emulator(cfg);
  emulator.Construct(nWidth, nHeight, 2, 2);
  emulator.Start();
  return 0;
}
