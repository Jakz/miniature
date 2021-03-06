#pragma once

#include "Common.h"

#include "SDL.h"
#include "SDL_image.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

#if !_WIN32
constexpr int32_t WIDTH = 320;
constexpr int32_t HEIGHT = 240;
#define MOUSE_ENABLED false
#else
constexpr int32_t WIDTH = 320;
constexpr int32_t HEIGHT = 240;
#define WINDOW_SCALE 2
#define MOUSE_ENABLED true
#endif

#if SDL_COMPILEDVERSION > 2000

struct Surface
{
  SDL_Surface* surface;
  SDL_Texture* texture;

  Surface(SDL_Surface* surface) : surface(surface) { }
  Surface(SDL_Surface* surface, SDL_Texture* texture) : surface(surface), texture(texture) { }

  Surface() : surface(nullptr), texture(nullptr) { }

  operator bool() const { return surface != nullptr; }

  void enableBlending() { assert(texture); SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND); }
  void releaseSurface() { if (surface) SDL_FreeSurface(surface); surface = nullptr; }

  void release()
  {
    releaseSurface();
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  void update() { SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch); }
  inline uint32_t& pixel(size_t index) { return pixels()[index]; }
  inline uint32_t* pixels() { return static_cast<uint32_t*>(surface->pixels); }
};

#if _WIN32
#define DEBUGGER false
#endif

#else

struct Surface
{
  SDL_Surface* surface;

  Surface() : surface(nullptr) { }
  Surface(SDL_Surface* surface) : surface(surface) { }

  operator bool() const { return surface != nullptr; }

  void enableBlending() { assert(surface); SDL_SetAlpha(surface, SDL_SRCALPHA, 0); }
  void releaseSurface() {  }

  void release()
  {
    assert(surface);
    SDL_FreeSurface(surface);
    surface = nullptr;
  }

  void update() { }
  inline uint32_t& pixel(size_t index) { return pixels()[index]; }
  inline uint32_t* pixels() { return static_cast<uint32_t*>(surface->pixels); }
};

#define SDL12

using SDL_Renderer = int;
using SDL_Window = int;
using SDL_AudioDeviceID = int;
#define SDL_OpenAudioDevice(x, y, w, s, z) SDL_OpenAudio(w, s)
#define SDL_PauseAudioDevice(_,y) SDL_PauseAudio(y)
#define SDL_CloseAudioDevice(_) SDL_CloseAudio()

#endif

template<typename EventHandler, typename Renderer>
class SDL
{
protected:
  EventHandler& eventHandler;
  Renderer& loopRenderer;

  SDL_PixelFormat* _format;
  SDL_Surface* _screen;
  SDL_Window* _window;
  SDL_Renderer* _renderer;

  bool willQuit;
  u32 ticks;
  float _lastFrameTicks;

  u32 frameRate;
  float ticksPerFrame;


public:
  SDL(EventHandler& eventHandler, Renderer& loopRenderer) : eventHandler(eventHandler), loopRenderer(loopRenderer),
    _screen(nullptr), _window(nullptr), _renderer(nullptr), willQuit(false), ticks(0)
  {
    setFrameRate(60);
  }

  Surface allocate(int width, int height);

  const SDL_PixelFormat* displayFormat() { return _format; }

  void setFrameRate(u32 frameRate)
  {
    this->frameRate = frameRate;
    this->ticksPerFrame = 1000 / (float)frameRate;
  }

  float lastFrameTicks() const { return _lastFrameTicks; }

  bool init();
  void deinit();
  void capFPS();

  void loop();
  void handleEvents();

  void exit() { willQuit = true; }

  void blit(const Surface& texture, const SDL_Rect& src, const SDL_Rect& dest);
  void blit(const Surface& texture, const SDL_Rect& src, int dx, int dy);
  void blit(const Surface& texture, int sx, int sy, int w, int h, int dx, int dy);
  void blit(const Surface& texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
  void blit(const Surface& texture, int dx, int dy);
  void blitToScreen(const Surface& texture, const SDL_Rect& rect);

  void clear(const Surface& texture, int r, int g, int b);
  void clear(int r, int g, int b);

  void rect(int x, int y, int w, int h, int r, int g, int b, int a);

  void release(const Surface& texture);

  SDL_Window* window() { return _window; }
  SDL_Renderer* renderer() { return _renderer; }
};

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::capFPS()
{
  u32 ticks = SDL_GetTicks();
  u32 elapsed = ticks - SDL::ticks;

  _lastFrameTicks = elapsed;

  if (elapsed < ticksPerFrame)
  {
    SDL_Delay(ticksPerFrame - elapsed);
    _lastFrameTicks = ticksPerFrame;
  }

  SDL::ticks = SDL_GetTicks();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        willQuit = true;
        break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
#if !defined(SDL12)
        if (!event.key.repeat)
#endif
          eventHandler.handleKeyboardEvent(event, event.type == SDL_KEYDOWN);
        break;

#if MOUSE_ENABLED
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
#if defined(WINDOW_SCALE)
        event.button.x /= WINDOW_SCALE;
        event.button.y /= WINDOW_SCALE;
#endif
        eventHandler.handleMouseEvent(event);
#endif
    }
  }
}
template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    LOGDD("Error on SDL_Init().\n");
    return false;
  }

  // SDL_WINDOW_FULLSCREEN
#if defined(WINDOW_SCALE)
  _window = SDL_CreateWindow("miniature", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_OPENGL);
#else
  _window = SDL_CreateWindow("miniature", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
#endif
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_RendererInfo info;
  SDL_GetRendererInfo(_renderer, &info);
  _format = SDL_AllocFormat(info.texture_formats[0]);

  return true;
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::loop()
{
  while (!willQuit)
  {
    loopRenderer.render();
    SDL_RenderPresent(_renderer);

    handleEvents();

    capFPS();
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  SDL_FreeFormat(_format);
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
Surface SDL<EventHandler, Renderer>::allocate(int width, int height)
{
  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, _format->Rmask, _format->Gmask, _format->Bmask, _format->Amask);
  SDL_Texture* texture = SDL_CreateTexture(_renderer, _format->format, SDL_TEXTUREACCESS_STREAMING, width, width);
  return { surface, texture };
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::blitToScreen(const Surface& surface, const SDL_Rect& rect)
{
  SDL_RenderCopy(_renderer, surface.texture, nullptr, &rect);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& src, const SDL_Rect& dest)
{
  SDL_RenderCopy(_renderer, surface.texture, &src, &dest);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(surface, { sx, sy, w, h }, dx, dy);
}


template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(surface.texture, &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::clear(int r, int g, int b)
{
  SDL_SetRenderDrawColor(_renderer, r, g, b, 255);
  SDL_RenderClear(_renderer);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::clear(const Surface& surface, int r, int g, int b)
{
  auto target = SDL_GetRenderTarget(_renderer);
  SDL_SetRenderTarget(_renderer, surface.texture);
  SDL_SetRenderDrawColor(_renderer, r, g, b, 255);
  SDL_RenderClear(_renderer);
  SDL_SetRenderTarget(_renderer, target);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::rect(int x, int y, int w, int h, int r, int g, int b, int a)
{
  SDL_SetRenderDrawColor(_renderer, r, g, b, a);
  SDL_Rect border = { x, y, w, h };
  SDL_RenderDrawRect(_renderer, &border);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::release(const Surface& surface)
{
  SDL_DestroyTexture(surface.texture);

  if (surface.surface)
    SDL_FreeSurface(surface.surface);
}

inline static SDL_Rect SDL_MakeRect(int x, int y, int w, int h) { return { x, y, w, h }; }

