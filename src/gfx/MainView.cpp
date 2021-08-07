#pragma once

#include "MainView.h"
#include "ViewManager.h"

using namespace ui;

u32* buffer = nullptr;
u64 offset = 0;

Surface screen;


void MainView::blitFramebuffer()
{
  for (int i = 0; i < display.width() * display.height(); ++i)
  {
    col_t nc = display.pixel(i);
    color_t c = display.ccc(nc);
    screen.pixel(i) = SDL_MapRGBA(gvm->displayFormat(), c.r, c.g, c.b, 255);
  }
}

MainView::MainView(ViewManager* gvm) : gvm(gvm), memory(), display(&memory)
{
  mouse = { -1, -1 };

  memory.clear();
}

void MainView::render()
{
  auto r = gvm->renderer();

  gvm->clear(20, 20, 20);

  if (!screen)
  {
    screen = gvm->allocate(display.width(), display.height());
  }

  memory.clear();

  display.rect(10, 10, 16, 16, rand() % 0xFFFF);

  //display.fill(display.ccc(255, 0, 0));

  
  blitFramebuffer();
  screen.update();

  gvm->blit(screen, 0, 0, display.width(), display.height(), 10, 10, display.width()*3, display.height()*3);
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