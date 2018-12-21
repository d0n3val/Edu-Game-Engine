#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "Event.h"
#include "SDL/include/SDL.h"

#include "mmgr/mmgr.h"

#define MAX_KEYS 300

ModuleInput::ModuleInput(bool start_enabled) : Module("Input", start_enabled)
{
	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	RELEASE_ARRAY(keyboard);
}

// Called before render is available
bool ModuleInput::Init(Config* config)
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called each loop iteration
update_status ModuleInput::PreUpdate(float dt)
{
	static SDL_Event event;

	mouse_motion_x = mouse_motion_y = 0;
	memset(windowEvents, false, WE_COUNT * sizeof(bool));
	
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
			{
				keyboard[i] = KEY_DOWN;
				App->editor->LogInputEvent(i, KEY_DOWN);
			}
			else if(keyboard[i] != KEY_REPEAT)
			{
				
				App->editor->LogInputEvent(i, KEY_REPEAT);
				keyboard[i] = KEY_REPEAT;
			}
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
			{
				keyboard[i] = KEY_UP;
				App->editor->LogInputEvent(i, KEY_UP);
			}
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	for(int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if (mouse_buttons[i] == KEY_DOWN)
		{
			App->editor->LogInputEvent(1000 + i, KEY_DOWN);
			App->editor->LogInputEvent(1000 + i, KEY_REPEAT);
			mouse_buttons[i] = KEY_REPEAT;
		}

		if (mouse_buttons[i] == KEY_UP)
		{
			mouse_buttons[i] = KEY_IDLE;
			App->editor->LogInputEvent(1000 + i, KEY_UP);
		}
	}

	mouse_wheel = 0;

	while(SDL_PollEvent(&event) != 0)
	{
		App->editor->HandleInput(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				windowEvents[WE_QUIT] = true;
			break;

			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					//case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					windowEvents[WE_HIDE] = true;
					break;

					//case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					windowEvents[WE_SHOW] = true;
					break;


					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						Event ev(Event::window_resize);
						ev.point2d.x = event.window.data1;
						ev.point2d.y = event.window.data2;
						App->BroadcastEvent(ev);
						break;
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
				mouse_buttons[event.button.button - 1] = KEY_DOWN;
			break;

			case SDL_MOUSEBUTTONUP:
				mouse_buttons[event.button.button - 1] = KEY_UP;
			break;

			case SDL_MOUSEMOTION:
				mouse_motion_x = event.motion.xrel;
				mouse_motion_y = event.motion.yrel;
				mouse_x = event.motion.x;
				mouse_y = event.motion.y;
			break;
			
			case SDL_MOUSEWHEEL:
				mouse_wheel = event.wheel.y;
			break;

			case SDL_DROPFILE: 
				Event ev(Event::file_dropped);
				ev.string.ptr = event.drop.file;
				App->BroadcastEvent(ev);
				SDL_free(event.drop.file);
				break;
		}
	}

	if(GetWindowEvent(EventWindow::WE_QUIT) == true || GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		return UPDATE_STOP;

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

// ---------
bool ModuleInput::GetWindowEvent(EventWindow ev) const
{
	return windowEvents[ev];
}

void ModuleInput::GetMouseMotion(int & x, int & y) const
{
	x = mouse_motion_x;
	y = mouse_motion_y;
}

void ModuleInput::GetMousePosition(int & x, int & y) const
{
	x = mouse_x;
	y = mouse_y;
}

int ModuleInput::GetMouseWheel() const
{
	return mouse_wheel;
}
