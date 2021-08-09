#pragma once

#include "MainView.h"
#include "ViewManager.h"

using namespace ui;

u32* buffer = nullptr;
u64 offset = 0;

Surface surface;


void MainView::blitFramebuffer()
{
  Screen& screen = machine.screen();
  
  for (int i = 0; i < machine.screen().width() * machine.screen().height(); ++i)
  {
    col_t nc = screen.pixel(i);
    color_t c = screen.ccc(nc);
    surface.pixel(i) = SDL_MapRGBA(gvm->displayFormat(), c.r, c.g, c.b, 255);
  }
}

std::string readableBytes(u64 amount)
{
  if (amount < KB1) return std::to_string(amount) + " bytes";
  else return std::to_string(amount / KB1) + " Kb";
}

MainView::MainView(ViewManager* gvm) : gvm(gvm)
{
  mouse = { -1, -1 };

  machine.reset();

  printf("Total memory: %s\n", readableBytes(Specs::MEMORY_SIZE).c_str());
  printf("Framebuffer size: %s\n", readableBytes(Specs::FRAMEBUFFER_SIZE_IN_BYTES).c_str());
  printf("Palettes size: %s\n", readableBytes(Specs::PALETTES_SIZE_IN_BYTES).c_str());
  printf("Sprite size: %s\n", readableBytes(Specs::SPRITE_SIZE_IN_BYTES).c_str());
  printf("Sprite map size: %s\n", readableBytes(Specs::SPRITE_MAP_SIZE_IN_BYTES).c_str());
  printf("Sprite info size: %s\n", readableBytes(Specs::SPRITE_INFO_SIZE_IN_BYTES).c_str());
  printf("Sprite infos size: %s\n", readableBytes(Specs::SPRITE_INFOS_SIZE_IN_BYTES).c_str());

  printf("\nMapped memory: %s\n", readableBytes(Specs::FRAMEBUFFER_SIZE_IN_BYTES + Specs::PALETTES_SIZE_IN_BYTES + Specs::SPRITE_MAP_SIZE_IN_BYTES + Specs::SPRITE_INFOS_SIZE_IN_BYTES).c_str());

}

void MainView::render()
{
  auto r = gvm->renderer();

  gvm->clear(20, 20, 20);

  auto& screen = machine.screen();

  if (!surface)
  {
    surface = gvm->allocate(screen.width(), screen.height());

    machine.spriteInfos()[0].flags |= SpriteFlag::Enabled;
    machine.palettes()[0][0] = machine.screen().ccc(255, 0, 0);
    machine.palettes()[0][1] = machine.screen().ccc(255, 180, 0);

    for (int y = 0; y < Specs::SPRITE_HEIGHT; ++y)
      for (int x = 0; x < Specs::SPRITE_WIDTH; ++x)
        machine.sprites()[0].set(x, y, rand() % 2);
  }

  screen.clear();

  /* for each sprite */
  for (s32 i = 0; i < Specs::SPRITE_INFO_SIZE; ++i)
  {
    SpriteInfo& info = machine.spriteInfos()[i];
    
    /* if sprite should be drawn */
    if (info.flags && SpriteFlag::Enabled)
    {
      Palette& palette = machine.palettes()[info.palette];
      Sprite sprite = machine.sprites()[info.index];

      /* draw it on framebuffer */
      for (int y = 0; y < Specs::SPRITE_HEIGHT; ++y)
        for (int x = 0; x < Specs::SPRITE_WIDTH; ++x)
        {
          auto fx = info.x + x, fy = info.y + y;
          col_idx_t c = sprite.get(x, y);
          //TODO: range check?

          screen.set(fx, fy, palette[c]);
        }
    }
  }


  /*screen.rect(10, 10, 16, 16, rand() % 0xFFFF);
  screen.line(1, 1, 30, 30, screen.ccc(255, 0, 0));
  screen.line(1, 2, 30, 31, screen.ccc(0, 255, 0));
  screen.line(1, 3, 30, 32, screen.ccc(0, 0, 255));*/

  blitFramebuffer();
  surface.update();

  gvm->blit(surface, 0, 0, screen.width(), screen.height(), 10, 10, screen.width()*3, screen.height()*3);
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