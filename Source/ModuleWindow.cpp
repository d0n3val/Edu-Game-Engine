#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "Event.h"
#include "Config.h"

ModuleWindow::ModuleWindow(bool start_enabled) : Module("Window", start_enabled)
{
}

// Destructor
ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init(Config* config)
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Create window
		screen_width = config->GetInt("Width", 1280);
		screen_height = config->GetInt("Height", 1024);

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 3.2
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		fullscreen = config->GetBool("Fullscreen", false);
		resizable = config->GetBool("Resizable", false);
		borderless = config->GetBool("Borderless", false);
		fullscreen_desktop = config->GetBool("Fullscreen Desktop", false);

		if(fullscreen == true)
			flags |= SDL_WINDOW_FULLSCREEN;

		if(resizable == true)
			flags |= SDL_WINDOW_RESIZABLE;

		if(borderless == true)
			flags |= SDL_WINDOW_BORDERLESS;

		if(fullscreen_desktop == true)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow(App->GetAppName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, flags);

		if(window == nullptr)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			screen_surface = SDL_GetWindowSurface(window);
		}
	}

	return ret;
}

bool ModuleWindow::Start(Config * config)
{
	SetDefaultIcon();

	std::string icon_file = config->GetString("Icon", "");
	if (icon_file.size() > 1)
		SetIcon(icon_file.c_str());

	SetBrightness(config->GetFloat("Brightness", 1.0f));

	// Force to trigger a chain of events to refresh aspect ratios	
	SDL_SetWindowSize(window, screen_width, screen_height);

	return true;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != nullptr)
		SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::Save(Config * config) const
{
	config->AddString("Icon", icon_file.c_str());
	config->AddFloat("Brightness", GetBrightness());
	config->AddInt("Width", GetWidth());
	config->AddInt("Height", this->GetHeight());
	config->AddBool("Fullscreen", IsFullscreen());
	config->AddBool("Resizable", IsResizable());
	config->AddBool("Borderless", IsBorderless());
	config->AddBool("Fullscreen Desktop", IsFullscreenDesktop());
}

void ModuleWindow::Load(Config * config)
{
	SetIcon(config->GetString("Icon", ""));
	SetBrightness(config->GetFloat("Brightness", 1.0f));
	SetWidth(config->GetInt("Width", 1280));
	SetHeigth(config->GetInt("Height", 1024));
	SetFullscreen(config->GetBool("Fullscreen", false));
	SetResizable(config->GetBool("Resizable", false));
	SetBorderless(config->GetBool("Borderless", false));
	SetFullScreenDesktop(config->GetBool("Fullscreen Desktop", false));
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

SDL_Window * ModuleWindow::GetWindow() const
{
	return window;
}

uint ModuleWindow::GetHeight() const
{
	return screen_height;
}

void ModuleWindow::SetWidth(uint width)
{
	SDL_SetWindowSize(window, width, GetHeight());
}

void ModuleWindow::SetHeigth(uint height)
{
	SDL_SetWindowSize(window, GetWidth(), height);
}

void ModuleWindow::GetMaxMinSize(uint & min_width, uint & min_height, uint & max_width, uint & max_height) const
{
	min_width = 640;
	min_height = 480;
	max_width = 3000;
	max_height = 2000;

	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0) 
	    LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
	else
	{
		max_width = dm.w;
		max_height = dm.h;
	}

	// Aparently this is only to gather what user setup in the SDl_SetWindowMaxumimSize()
	//SDL_GetWindowMinimumSize(window, (int*) &min_width, (int*) &min_height);
	//SDL_GetWindowMaximumSize(window, (int*) &max_width, (int*) &max_height);
}

uint ModuleWindow::GetRefreshRate() const
{
	uint ret = 0;

	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
		LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
	else
		ret = dm.refresh_rate;

	return ret;
}

void ModuleWindow::ReceiveEvent(const Event& event)
{
	switch (event.type)
	{
		case Event::window_resize:
			screen_width = event.point2d.x;
			screen_height = event.point2d.y;
		break;
	}
}

bool ModuleWindow::IsFullscreen() const
{
	return fullscreen;
}

bool ModuleWindow::IsResizable() const
{
	return resizable;
}

bool ModuleWindow::IsBorderless() const
{
	return borderless;
}

bool ModuleWindow::IsFullscreenDesktop() const
{
	return fullscreen_desktop;
}

float ModuleWindow::GetBrightness() const
{
	return SDL_GetWindowBrightness(window);
}

void ModuleWindow::SetFullscreen(bool set)
{
	if (set != fullscreen)
	{
		fullscreen = set;
		if (fullscreen == true)
		{
			if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) != 0)
				LOG("Could not switch to fullscreen: %s\n", SDL_GetError());
			fullscreen_desktop = false;
			SDL_Log("this is a test");
		}
		else
		{
			if (SDL_SetWindowFullscreen(window, 0) != 0)
				LOG("Could not switch to windowed: %s\n", SDL_GetError());
		}
	}
}

void ModuleWindow::SetResizable(bool set)
{
	// cannot be changed while the program is running, but we can save the change
	resizable = set;
}

void ModuleWindow::SetBorderless(bool set)
{
	if (set != borderless && fullscreen == false && fullscreen_desktop == false )
	{
		borderless = set;
		SDL_SetWindowBordered(window, (SDL_bool)!borderless);
	}
}

void ModuleWindow::SetFullScreenDesktop(bool set)
{
	if (set != fullscreen_desktop)
	{
		fullscreen_desktop = set;
		if (fullscreen_desktop == true)
		{
			if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
				LOG("Could not switch to fullscreen desktop: %s\n", SDL_GetError());
			fullscreen = false;
		}
		else
		{
			if (SDL_SetWindowFullscreen(window, 0) != 0)
				LOG("Could not switch to windowed: %s\n", SDL_GetError());
		}
	}
}

void ModuleWindow::SetBrightness(float set)
{
	CAP(set);
	if(SDL_SetWindowBrightness(window, set) != 0)
		LOG("Could not change window brightness: %s\n", SDL_GetError());
}

const char * ModuleWindow::GetIcon() const
{
	return icon_file.c_str();
}

void ModuleWindow::SetIcon(const char * file)
{
	if (file != nullptr && file != icon_file)
	{
		icon_file = file;

		SDL_Surface *surface = SDL_LoadBMP_RW(App->fs->Load(file), 1);
		SDL_SetWindowIcon(window, surface);
		SDL_FreeSurface(surface);
	}
}

uint ModuleWindow::GetWidth() const
{
	return screen_width;
}

void ModuleWindow::SetDefaultIcon()
{
	icon_file = "*default*";

	#include "default_icon.h"

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*) image32, 32, 32, 32, sizeof(unsigned long) * 32,0,0,0,0);
	if(!surface)
		LOG("Could not set default icon for app: %s\n", SDL_GetError());
	SDL_SetWindowIcon(window, surface);
	SDL_FreeSurface(surface);
}