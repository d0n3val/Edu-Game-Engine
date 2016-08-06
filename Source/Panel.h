#ifndef __PANEL_H__
#define __PANEL_H__

// Base class for all possible Editor Panels
#include "Globals.h"
#include <string>
#include "SDL/include/SDL_scancode.h"

#define IMGUI_BLUE ImVec4(0,0,1,1)
#define IMGUI_GREEN ImVec4(0,1,0,1)
#define IMGUI_YELLOW ImVec4(1,1,0,1)
#define IMGUI_RED ImVec4(1,0,0,1)
#define IMGUI_WHITE ImVec4(1,1,1,1)

class Panel
{
public:
	Panel(const char* name, SDL_Scancode shortcut = SDL_SCANCODE_UNKNOWN);
	virtual ~Panel();

	void SwitchActive();
	bool IsActive() const;

	SDL_Scancode GetShortCut() const;
	virtual void Draw() = 0;

public:
	bool active = false;

protected:
	std::string name;
	SDL_Scancode shortcut = SDL_SCANCODE_UNKNOWN;
};

#endif // __PANEL_H__