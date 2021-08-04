#pragma once

#include "MainView.h"
#include "ViewManager.h"

using namespace ui;

u32* buffer = nullptr;
u64 offset = 0;

static constexpr s32 SW = 128;
static constexpr s32 SH = 128;

Surface screen;

static_assert((SW* SH) % 2 == 0);

static constexpr u32 KB64 = 2 ^ 16;
static constexpr u32 KB128 = 2 ^ 17;
static constexpr u32 KB256 = 2 ^ 18;

static constexpr u32 RED_SHIFT = 10, GREEN_SHIFT = 5, BLUE_SHIFT = 0;
static constexpr u32 COLOR_MASK = 0b11111, COLOR_SHIFT = 3;

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


u8 memory[KB128];

u16* vram = (u16*)memory;

void MainView::convertFrom15bbpTo32bbp(u16 source, u32* dest)
{
  
  u16 color = source;

  // X RRRRR GGGGG BBBBB
  color_t c = ccc(color);

  *dest = SDL_MapRGBA(gvm->displayFormat(), c.r, c.g, c.b, 255);
}

MainView::MainView(ViewManager* gvm) : gvm(gvm)
{
  mouse = { -1, -1 };

  std::fill(memory, memory + KB128, 0);
}

void MainView::render()
{
  auto r = gvm->renderer();

  gvm->clear(20, 20, 20);

  if (!screen)
  {
    screen = gvm->allocate(SW, SH);
  }

  //gvm->clear(screen, 255, 255, 255);
  
  convertFrom15bbpTo32bbp(ccc(255, 0, 0), &screen.pixel(160));
  convertFrom15bbpTo32bbp(ccc(0, 255, 0), &screen.pixel(161));
  convertFrom15bbpTo32bbp(ccc(0, 0, 255), &screen.pixel(162));

  screen.update();

  gvm->blit(screen, 0, 0, SW, SH, 10, 10, SW*2, SH*2);
}

void MainView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: gvm->exit(); break;
    }
  }
}

void MainView::handleMouseEvent(const SDL_Event& event)
{
  if (event.type == SDL_MOUSEMOTION)
  {
    mouse.x = event.motion.x;
    mouse.y = event.motion.y;
  }
}