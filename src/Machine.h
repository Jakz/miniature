#pragma once

#include "Common.h"

#include "FlagSet.h"

static constexpr s32 KB1 = 1024;
static constexpr s32 KB64 = 1 << 16;
static constexpr s32 KB128 = 1 << 17;
static constexpr s32 KB256 = 1 << 18;

using addr_t = u32;
using col_t = u16;
using col_idx_t = u32;
using col_idx_pair_t = u32;

enum class SpriteFlag : u32
{
  Enabled = 0x00000001,
};

enum class SpriteSize { _8, _16, _24, _32, _48, _64 };

struct SpriteInfo
{
  flag_set<SpriteFlag> flags;
  s16 x, y;
  u8 index;
  u8 palette; // : 4
};

struct Specs
{
  static constexpr addr_t MEMORY_SIZE = KB128;
  static constexpr s32 SCREEN_WIDTH = 128;
  static constexpr s32 SCREEN_HEIGHT = 128;

  static constexpr s32 BITS_PER_COLOR_COMPONENT = 5;
  
  static constexpr s32 PALETTE_SIZE = 16;
  static constexpr s32 PALETTE_COUNT = 16;

  static constexpr s32 SPRITE_WIDTH = 8;
  static constexpr s32 SPRITE_HEIGHT = 8;
  static constexpr s32 SPRITE_MAP_WIDTH = 16;
  static constexpr s32 SPRITE_MAP_SIZE = 256;
  static constexpr s32 SPRITE_MAPS_COUNT = 2;

  static constexpr s32 SPRITE_INFO_SIZE = 128;
  static constexpr s32 SPRITE_INFO_SIZE_IN_BYTES = sizeof(SpriteInfo);
  static constexpr s32 SPRITE_INFOS_SIZE_IN_BYTES = SPRITE_INFO_SIZE * SPRITE_INFO_SIZE_IN_BYTES;

  static constexpr s32 BITS_PER_SPRITE_COLOR = 4;
  static constexpr s32 BITS_PER_SPRITE_PALETTE = 4;

  static constexpr s32 FRAMEBUFFER_SIZE_IN_BYTES = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(col_t);
  static constexpr s32 PALETTES_SIZE_IN_BYTES = PALETTE_SIZE * PALETTE_COUNT * sizeof(col_t);
  static constexpr s32 SPRITE_ROW_SIZE_IN_BYTES = SPRITE_WIDTH * (BITS_PER_SPRITE_COLOR / 8.0f);
  static constexpr s32 SPRITE_SIZE_IN_BYTES = SPRITE_HEIGHT * SPRITE_ROW_SIZE_IN_BYTES;
  static constexpr s32 SPRITE_MAP_SIZE_IN_BYTES = SPRITE_SIZE_IN_BYTES * SPRITE_MAP_SIZE;

  static constexpr s32 RED_SHIFT = BITS_PER_COLOR_COMPONENT * 2;
  static constexpr s32 GREEN_SHIFT = BITS_PER_COLOR_COMPONENT * 1;
  static constexpr s32 BLUE_SHIFT = 0;

  static constexpr s32 COLOR_MASK = (1 << BITS_PER_COLOR_COMPONENT) - 1;
  static constexpr s32 COLOR_SHIFT = 8 - BITS_PER_COLOR_COMPONENT;
  
  static_assert(BITS_PER_COLOR_COMPONENT * 3 <= sizeof(col_t) * 8);
};

struct Address
{
  static constexpr addr_t VRAM = Specs::MEMORY_SIZE - Specs::FRAMEBUFFER_SIZE_IN_BYTES;
  static constexpr addr_t PALETTES = VRAM - Specs::PALETTES_SIZE_IN_BYTES;
  static constexpr addr_t SPRITE_MAP = PALETTES - Specs::SPRITE_MAP_SIZE_IN_BYTES;
  static constexpr addr_t SPRITE_INFOS = SPRITE_MAP - Specs::SPRITE_INFOS_SIZE_IN_BYTES;
};

struct Color
{
  static constexpr col_t BLACK = 0x8000;
  static constexpr col_t WHITE = 0xFFFF;

  static constexpr col_t TRANSPARENT = 0x0000;
  static constexpr col_t OPAQUE_FLAG = 0x8000;

  static constexpr bool isOpaque(col_t c) { return c & OPAQUE_FLAG; }
  static constexpr bool isTransparent(col_t c) { return !isOpaque(c); }

  static constexpr col_t ccc(u8 r, u8 g, u8 b)
  {
    //TODO: not using full brightness colors
    return OPAQUE_FLAG |
      ((r >> (Specs::COLOR_SHIFT)) << Specs::RED_SHIFT) |
      ((g >> (Specs::COLOR_SHIFT)) << Specs::GREEN_SHIFT) |
      ((b >> (Specs::COLOR_SHIFT)) << Specs::BLUE_SHIFT);
  }

  static color_t ccc(u16 c)
  {
    u8 r = ((c >> Specs::RED_SHIFT) & Specs::COLOR_MASK) << Specs::COLOR_SHIFT;
    u8 g = ((c >> Specs::GREEN_SHIFT) & Specs::COLOR_MASK) << Specs::COLOR_SHIFT;
    u8 b = ((c >> Specs::BLUE_SHIFT) & Specs::COLOR_MASK) << Specs::COLOR_SHIFT;
    return { r, g, b };
  }
};



struct Sprite
{
private:
  std::array<u8, Specs::SPRITE_SIZE_IN_BYTES> data;

public:

  void set(coord_t i, col_idx_t color)
  {
    auto b = i / 2, o = i % 2;

    if (o) data[b] = (data[b] & 0xF0) | (color);
    else data[b] = (data[b] & 0x0F) | (color << 4);
  }

  void set(coord_t x, coord_t y, col_idx_t color)
  {
    set(x + y * Specs::SPRITE_WIDTH, color);
  }

  void setRow(coord_t y, const std::array<col_idx_t, Specs::SPRITE_WIDTH>& data)
  {
    for (coord_t x = 0; x < Specs::SPRITE_WIDTH; ++x)
      set(x, y, data[x]);
  }

  void setRowString(coord_t y, const std::string& data)
  {
    assert(data.size() == Specs::SPRITE_WIDTH);
    for (coord_t x = 0; x < Specs::SPRITE_WIDTH; ++x)
      set(x, y, (data[x] >= '0' || data[x] <= '9') ? (data[x] - '0') : (data[x] - 'A'));
  }
  
  void setString(const std::string& data)
  {
    assert(data.size() == Specs::SPRITE_HEIGHT * Specs::SPRITE_WIDTH);
    for (coord_t i = 0; i < Specs::SPRITE_HEIGHT * Specs::SPRITE_WIDTH; ++i)
      set(i, (data[i] >= '0' || data[i] <= '9') ? (data[i] - '0') : (data[i] - 'A'));
  }

  col_idx_t get(coord_t x, coord_t y)
  {
    u8* base = row(y) + (x / 2);

    if (x % 2 == 0)
      return (*base >> 4) & 0x0F;
    else
      return *base & 0x0F;
  }

  u8* row(coord_t y) { return data.data() + y * Specs::SPRITE_ROW_SIZE_IN_BYTES; }
};

using Palette = std::array<col_t, Specs::PALETTE_SIZE>;
using Palettes = std::array<Palette, Specs::PALETTE_COUNT>;
using SpriteMap = std::array<Sprite, Specs::SPRITE_MAP_SIZE>;
using SpriteInfos = std::array<SpriteInfo, Specs::SPRITE_INFO_SIZE>;

class Memory
{
public:
  constexpr addr_t size() const { return Specs::MEMORY_SIZE; }

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

using coord_t = s32;

class Screen
{
private:
  Memory* memory;

  const col_t* framebuffer() const { return memory->addr<col_t>(Address::VRAM); }
  col_t* framebuffer() { return memory->addr<col_t>(Address::VRAM); }

public:
  Screen(Memory* memory) : memory(memory)
  {
    static_assert((Specs::SCREEN_WIDTH * Specs::SCREEN_HEIGHT) % 2 == 0);
  }

public:
  auto width() const { return Specs::SCREEN_WIDTH; }
  auto height() const { return Specs::SCREEN_HEIGHT; }

  col_t pixel(addr_t i) const { return framebuffer()[i]; }

  void clear();
  void fill(col_t color);
  
  void set(coord_t x, coord_t y, col_t color);

  void rect(coord_t x, coord_t y, coord_t w, coord_t h, col_t color);
  void line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, col_t color);
};


class Machine
{
private:
  Memory _memory;
  Screen _screen;

public:
  Machine();

  Memory& memory() { return _memory; }
  Screen& screen() { return _screen; }
  Palettes& palettes() { return *_memory.addr<Palettes>(Address::PALETTES); }
  SpriteMap& sprites() { return *_memory.addr<SpriteMap>(Address::SPRITE_MAP); }
  SpriteInfos& spriteInfos() { return *_memory.addr<SpriteInfos>(Address::SPRITE_INFOS); }

  void reset();
};