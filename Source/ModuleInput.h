#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include "Point.h"

#include "SDL/include/SDL_scancode.h"

#define NUM_MOUSE_BUTTONS 5

enum EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class ModuleInput : public Module
{

public:

	ModuleInput(bool start_enabled = true);

	// Destructor
	virtual ~ModuleInput();

	// Called before render is available
	bool Init(Config* config = nullptr);

	// Called each loop iteration
	update_status PreUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	// Check key states
	KeyState GetKey(int id) const
	{
		return keyboard[id];
	}

	KeyState GetMouseButton(int id) const
	{
		return mouse_buttons[id - 1];
	}

	// Check for window events last frame
	bool GetWindowEvent(EventWindow code) const;

	// Get mouse / axis position
	const iPoint& GetMouseMotion() const;
	const iPoint& GetMousePosition() const;
	int GetMouseWheel() const;

private:
	bool		windowEvents[WE_COUNT];
	KeyState*	keyboard = nullptr;
	KeyState	mouse_buttons[NUM_MOUSE_BUTTONS];
	iPoint mouse_motion;
	iPoint mouse;
	int mouse_wheel;
};

#endif // __MODULEINPUT_H__