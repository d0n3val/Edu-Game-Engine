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
	void ReceiveEvent(const Event& event) override;

	void Save(Config* config) const override;
	void Load(Config* config) override;

	void SetTitle(const char* title);

	SDL_Window* GetWindow() const;

	uint GetWidth() const;
	void SetDefaultIcon();
	uint GetHeight() const;
	void SetWidth(uint width);
	void SetHeigth(uint height);
	void GetMaxMinSize(uint& min_width, uint& min_height, uint& max_width, uint& max_height) const;
	uint GetRefreshRate() const;
	void OnResize(int width, int height);

	bool IsFullscreen() const;
	bool IsResizable() const;
	bool IsBorderless() const;
	bool IsFullscreenDesktop() const;
	float GetBrightness() const;

	void SetFullscreen(bool set);
	void SetResizable(bool set);
	void SetBorderless(bool set);
	void SetFullScreenDesktop(bool set);
	void SetBrightness(float set);

	const char* GetIcon() const;
	void SetIcon(const char* file);

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
	std::string icon_file;

};

#endif // __ModuleWindow_H__