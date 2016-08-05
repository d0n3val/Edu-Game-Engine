#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include "SDL/include/SDL.h"

struct SDL_Window;
struct SDL_Surface;

class ModuleWindow : public Module
{
public:

	ModuleWindow(bool start_enabled = true);

	// Destructor
	virtual ~ModuleWindow();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	bool CleanUp() override;

	void SetTitle(const char* title);

	SDL_Window* GetWindow() const;

	uint GetWidth() const;
	uint GetHeigth() const;
	void OnResize(int width, int height);

	bool IsFullscreen() const;
	bool IsResizable() const;
	bool IsBorderless() const;
	bool IsFullscreenDesktop() const;

private:
	//The window we'll be rendering to
	SDL_Window* window = nullptr;

	//The surface contained by the window
	SDL_Surface* screen_surface = nullptr;

	uint screen_width = 1280;
	uint screen_height = 1024;
	bool fullscreen = false;
	bool resizable = false;
	bool borderless = false;
	bool fullscreen_desktop = false;

};

#endif // __ModuleWindow_H__