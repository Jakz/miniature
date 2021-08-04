#pragma once

#include "ViewManager.h"
#include "Common.h"

struct ObjectGfx;

namespace ui
{
  class MainView : public View
  {
  private:
    ViewManager* gvm;
    point_t mouse;
    
    void convertFrom15bbpTo32bbp(u16 source, u32* dest);

  public:
    MainView(ViewManager* gvm);

    void render() override;
    void handleKeyboardEvent(const SDL_Event& event) override;
    void handleMouseEvent(const SDL_Event& event) override;
  };
}
