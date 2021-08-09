#include "Machine.h"

#define CHECK(x)

static_assert(sizeof(SpriteMap) == Specs::SPRITE_MAP_SIZE_IN_BYTES);
static_assert(sizeof(Sprite) == Specs::SPRITE_SIZE_IN_BYTES);
static_assert(sizeof(Palettes) == Specs::PALETTES_SIZE_IN_BYTES);


void Screen::set(coord_t x, coord_t y, col_t color)
{
  framebuffer()[x + y * width()] = color;
}

void Screen::clear()
{
  fill(Color::BLACK);
}

void Screen::fill(col_t color)
{
  std::fill(framebuffer(), framebuffer() + width() * height(), color);
}

void Screen::rect(coord_t x, coord_t y, coord_t w, coord_t h, col_t color)
{
  CHECK(x >= 0 && x + w < width() && y >= 0 && y + h < height());

  for (auto j = y; j < y + h; ++j)
  {
    auto start = framebuffer() + j * width() + x;
    std::fill(start, start + w, color);
  }
}

void Screen::line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, col_t color)
{
  // vertical
  if (y0 == y1)
  {
    if (x0 > x1) std::swap(x0, x1);

    for (coord_t x = x0; x <= x1; ++x)
      set(x, y0, color);
  }
  // horizontal
  else if (x0 == x1)
  {
    if (y0 > y1) std::swap(y0, y1);

    for (coord_t y = y0; y <= y1; ++y)
      set(x0, y, color);
  }
  else
  {
    coord_t dx = std::abs(x1 - x0);
    coord_t sx = x0 < x1 ? 1 : -1;
    coord_t dy = -std::abs(y1 - y0);
    coord_t sy = y0 < y1 ? 1 : -1;
    coord_t err = dx + dy;

    while (true)
    {
      set(x0, y0, color);

      if (x0 == x1 && y0 == y1)
        break;

      coord_t err2 = 2 * err;

      if (err2 >= dy)
      {
        err += dy;
        x0 += sx;
      }

      if (err2 <= dx)
      {
        err += dx;
        y0 += sy;
      }
    }
  }
}



Machine::Machine() : _screen(&_memory)
{

}

void Machine::reset()
{
  _memory.clear();
}