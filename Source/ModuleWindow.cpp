#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
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
	// Set App Icon
	SDL_Surface *surface = SDL_LoadBMP_RW(App->fs->Load(config->GetString("Icon", "")), 1);
	SDL_SetWindowIcon(window, surface);
	SDL_FreeSurface(surface);

	return true;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != nullptr)
	{
		SDL_DestroyWindow(window);
	}

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

SDL_Window * ModuleWindow::GetWindow() const
{
	return window;
}

uint ModuleWindow::GetHeigth() const
{
	return screen_height;
}

void ModuleWindow::OnResize(int width, int height)
{
	screen_width = width;
	screen_height = height;
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

uint ModuleWindow::GetWidth() const
{
	return screen_width;
}
