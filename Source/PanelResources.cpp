#include "PanelResources.h"
#include "ModuleInput.h"
#include "Imgui/imgui.h"

// ---------------------------------------------------------
PanelResources::PanelResources() : Panel("Resources", SDL_SCANCODE_F5)
{
	width = 325;
	height = 500;
	posx = 2;
	posy = 500;
}

// ---------------------------------------------------------
PanelResources::~PanelResources()
{}

// ---------------------------------------------------------
void PanelResources::Draw()
{
    ImGui::Begin("Resources", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing );
	ImGui::Text("Hello world");
    ImGui::End();
}