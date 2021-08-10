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
    color_t c = Color::ccc(nc);
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

    auto& info = machine.spriteInfos()[0];
    info.flags |= SpriteFlag::Enabled;
    info.flags |= SpriteFlag::FlippedY;
    info.x = 10;
    info.y = 10;
    info.width = SpriteSize::_16;
    info.height = SpriteSize::_16;

    machine.palettes()[0][1] = Color::ccc(200, 76, 12);
    machine.palettes()[0][2] = Color::ccc(252, 188, 176);
    machine.palettes()[0][3] = Color::ccc(0, 0, 0);

    Sprite& sprite = machine.sprites()[0];
    sprite.setRow(0, { 0, 0, 0, 0, 0, 0, 1, 1 });
    sprite.setRow(1, { 0, 0, 0, 0, 0, 1, 1, 1 });
    sprite.setRow(2, { 0, 0, 0, 0, 1, 1, 1, 1 });
    sprite.setRow(3, { 0, 0, 0, 3, 3, 1, 1, 1 });
    sprite.setRow(4, { 0, 0, 1, 1, 2, 3, 1, 1 });
    sprite.setRow(5, { 0, 1, 1, 1, 2, 3, 1, 1 });
    sprite.setRow(6, { 0, 1, 1, 1, 2, 3, 3, 3 });
    sprite.setRowString(7, "11112321");

    Sprite& sprite1 = machine.sprites()[1];
    sprite1.setRowString(0, "11000000");
    sprite1.setRowString(1, "11100000");
    sprite1.setRowString(2, "11110000");
    sprite1.setRowString(3, "11133000");
    sprite1.setRowString(4, "11321100");
    sprite1.setRowString(5, "11321110");
    sprite1.setRowString(6, "33321110");
    sprite1.setRowString(7, "12321111");

    Sprite& sprite2 = machine.sprites()[16];
    sprite2.setString("11112221" "11111111" "01111222" "00002222" "00002222" "00033222" "00033322" "00003330");

    Sprite& sprite3 = machine.sprites()[17];
    sprite3.setString("12221111" "11111111" "22211110" "22220000" "22223300" "22333330" "23333330" "03333300" );
  }

  screen.fill(Color::ccc(27, 89, 156));
  screen.rasterizeSprites();

  static u64 counter = 0;

  ++counter;

  if (counter % 4 == 0)
  {
    machine.spriteInfos()[0].flags.flip(SpriteFlag::FlippedY);
    ++machine.spriteInfos()[0].x;
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