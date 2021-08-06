#pragma once

#include "ViewManager.h"
#include "Common.h"
#include "Machine.h"

struct ObjectGfx;

namespace ui
{
  class MainView : public View
  {
  private:
    ViewManager* gvm;
    point_t mouse;

    Memory memory;
    Display display;
    
    void blitFramebuffer();

  public:
    MainView(ViewManager* gvm);

    void render() override;
    void handleKeyboardEvent(const SDL_Event& event) override;
    void handleMouseEvent(const SDL_Event& event) override;
  };
}
