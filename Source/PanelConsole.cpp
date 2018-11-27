#include "PanelConsole.h"
#include "ModuleInput.h"

// ---------------------------------------------------------
PanelConsole::PanelConsole() : Panel("Console", SDL_SCANCODE_1)
{
	width = 658;
	height = 105;
	posx = 325;
	posy = 919;
}

// ---------------------------------------------------------
PanelConsole::~PanelConsole()
{}

// ---------------------------------------------------------
void PanelConsole::Clear() 
{ 
	Buf.clear(); 
}

// ---------------------------------------------------------
void PanelConsole::AddLog(const char* entry)
{
	Buf.appendf(entry);
	ScrollToBottom = true;
}

// ---------------------------------------------------------
void PanelConsole::Draw()
{
    ImGui::Begin("Console", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing );
    ImGui::TextUnformatted(Buf.begin());
    if (ScrollToBottom)
        ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;
    ImGui::End();
}