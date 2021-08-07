#pragma once

#include "Common.h"

static constexpr u32 KB64 = 2 << 16;
static constexpr u32 KB128 = 2 << 17;
static constexpr u32 KB256 = 2 << 18;

static constexpr u32 RED_SHIFT = 10, GREEN_SHIFT = 5, BLUE_SHIFT = 0;
static constexpr u32 COLOR_MASK = 0b11111, COLOR_SHIFT = 3;

using addr_t = u32;
using col_t = u16;

struct Address
{
  static constexpr addr_t VRAM = 0;
};

class Memory
{
public:
  constexpr addr_t size() const { return KB128; }

private:
  u8* memory;

public:
  Memory() : memory(new u8[size()])
  {

  }

  ~Memory()
  {
    delete[] memory;
  }

  void clear()
  {
    std::fill(memory, memory + size(), 0);
  }

  void* raw() { return memory; }

  template<typename T> T* addr(addr_t addr) { return reinterpret_cast<T*>(&memory[addr]); }
  template<typename T> const T* addr(addr_t addr) const { return reinterpret_cast<T*>(&memory[addr]); }

  u8& byte(addr_t addr) { return reinterpret_cast<u8&>(memory[addr]); }
  u16& word(addr_t addr) { return reinterpret_cast<u16&>(memory[addr]); }
  u32& dword(addr_t addr) { return reinterpret_cast<u32&>(memory[addr]); }

  const u8& byte(addr_t addr) const { return reinterpret_cast<const u8&>(memory[addr]); }
  const u16& word(addr_t addr) const { return reinterpret_cast<const u16&>(memory[addr]); }
  const u32& dword(addr_t addr) const { return reinterpret_cast<const u32&>(memory[addr]); }
};

class Display
{
private:
  Memory* memory;

  const col_t* framebuffer() const { return memory->addr<col_t>(Address::VRAM); }
  col_t* framebuffer() { return memory->addr<col_t>(Address::VRAM); }

public:
  Display(Memory* memory) : memory(memory)
  {
    static_assert((WIDTH * HEIGHT) % 2 == 0);
  }

public:
  static constexpr s32 WIDTH = 128;
  static constexpr s32 HEIGHT = 128;

  auto width() const { return WIDTH; }
  auto height() const { return HEIGHT; }

  col_t pixel(addr_t i) const { return framebuffer()[i]; }

  constexpr u16 ccc(u8 r, u8 g, u8 b)
  {
    return
      ((r >> (COLOR_SHIFT)) << RED_SHIFT) |
      ((g >> (COLOR_SHIFT)) << GREEN_SHIFT) |
      ((b >> (COLOR_SHIFT)) << BLUE_SHIFT);
  }

  color_t ccc(u16 c)
  {
    u8 r = ((c >> RED_SHIFT) & COLOR_MASK) << COLOR_SHIFT;
    u8 g = ((c >> GREEN_SHIFT) & COLOR_MASK) << COLOR_SHIFT;
    u8 b = ((c >> BLUE_SHIFT) & COLOR_MASK) << COLOR_SHIFT;
    return { r, g, b };
  }

  void fill(col_t color);
  void set(u32 x, u32 y, col_t color);
  void rect(u32 x, u32 y, u32 w, u32 h, col_t color);
};
