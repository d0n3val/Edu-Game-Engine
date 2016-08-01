#include "PanelProperties.h"
#include "Imgui/imgui.h"
#include "GameObject.h"

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties", SDL_SCANCODE_F3)
{}

// ---------------------------------------------------------
PanelProperties::~PanelProperties()
{}

// ---------------------------------------------------------
void PanelProperties::Draw()
{
    ImGui::Begin("Properties", &active, ImGuiWindowFlags_NoFocusOnAppearing );
    ImGui::End();
}
