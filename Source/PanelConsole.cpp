#include "PanelConsole.h"
#include "ModuleInput.h"

// ---------------------------------------------------------
PanelConsole::PanelConsole() : Panel("Console", SDL_SCANCODE_F1)
{}

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
	Buf.append(entry);
	ScrollToBottom = true;
}

// ---------------------------------------------------------
void PanelConsole::Draw()
{
    ImGui::Begin("Console", &active, ImGuiWindowFlags_NoFocusOnAppearing );
    ImGui::TextUnformatted(Buf.begin());
    if (ScrollToBottom)
        ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;
    ImGui::End();
}