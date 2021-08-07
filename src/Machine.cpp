#include "Machine.h"

#define CHECK(x)

void Display::set(u32 x, u32 y, col_t color)
{
  framebuffer()[x + y * width()] = color;
}

void Display::fill(col_t color)
{
  std::fill(framebuffer(), framebuffer() + width() * height(), color);
}

void Display::rect(u32 x, u32 y, u32 w, u32 h, col_t color)
{
  CHECK(x >= 0 && x + w < width() && y >= 0 && y + h < height());

  for (auto j = y; j < y + h; ++j)
  {
    auto start = framebuffer() + j * width() + x;
    std::fill(start, start + w, color);
  }
}
