#include "Machine.h"

#define CHECK(x)

static_assert(sizeof(SpriteMap) == Specs::SPRITE_MAP_SIZE_IN_BYTES);
static_assert(sizeof(Sprite) == Specs::SPRITE_SIZE_IN_BYTES);
static_assert(sizeof(Palettes) == Specs::PALETTES_SIZE_IN_BYTES);

coord_t SpriteInfo::pixelWidth() const { return Specs::SPRITE_WIDTH * ((coord_t)width + 1); }
coord_t SpriteInfo::pixelHeight() const { return Specs::SPRITE_HEIGHT * ((coord_t)height + 1); }



Screen::Screen(Machine* machine) : machine(machine), memory(&machine->memory())
{
  static_assert((Specs::SCREEN_WIDTH * Specs::SCREEN_HEIGHT) % 2 == 0);
}


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

void Screen::rasterizeSprites()
{
  /* for each sprite */
  for (s32 i = 0; i < Specs::SPRITE_INFO_SIZE; ++i)
  {
    const SpriteInfo& info = machine->spriteInfos()[i];
    const bool flippedY = info.flags && SpriteFlag::FlippedY;
    const bool flippedX = info.flags && SpriteFlag::FlippedX;

    const coord_t ww = coord_t(info.width), hh = coord_t(info.height);

    /* if sprite should be drawn */
    if (info.flags && SpriteFlag::Enabled)
    {
      Palette& palette = machine->palettes()[info.palette];

      /* handle multi-tile sprites */
      for (coord_t ty = 0; ty < ww; ++ty)
      {
        for (coord_t tx = 0; tx < hh; ++tx)
        {
          coord_t iy = flippedX ? (hh - 1 - ty) : ty;
          coord_t ix = flippedY ? (ww - 1 - tx) : tx;

          Sprite sprite = machine->sprites()[info.index + ix + iy * Specs::SPRITE_MAP_WIDTH];

          /* draw it on framebuffer */
          for (int y = 0; y < Specs::SPRITE_HEIGHT; ++y)
            for (int x = 0; x < Specs::SPRITE_WIDTH; ++x)
            {
              auto fx = info.x + x + tx * Specs::SPRITE_WIDTH, fy = info.y + y + ty * Specs::SPRITE_HEIGHT;

              coord_t sy = flippedX ? (Specs::SPRITE_HEIGHT - 1 - y) : y;
              coord_t sx = flippedY ? (Specs::SPRITE_WIDTH - 1 - x) : x;

              col_idx_t c = sprite.get(sx, sy);
              //TODO: range check?

              col_t color = palette[c];

              if (Color::isOpaque(color))
                set(fx, fy, color);
            }
        }
      }


    }
  }

}



Machine::Machine() : _screen(this)
{

}

void Machine::reset()
{
  _memory.clear();
}