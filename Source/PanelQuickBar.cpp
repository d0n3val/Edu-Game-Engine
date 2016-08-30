#include "PanelQuickBar.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "Imgui/imgui.h"

// ---------------------------------------------------------
PanelQuickBar::PanelQuickBar() : Panel("About")
{
	width = 300;
	height = 28;
	posx = 325;
	posy = 20;
}

// ---------------------------------------------------------
PanelQuickBar::~PanelQuickBar()
{}

// ---------------------------------------------------------
void PanelQuickBar::Draw()
{
    ImGui::Begin("Quick Bar", &active, 
		ImGuiWindowFlags_NoMove |            
		ImGuiWindowFlags_NoTitleBar |            
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse );

	Application::State state = App->GetState();

	if (state != Application::play && state != Application::pause)
	{
		if (ImGui::Button("PLAY", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
			App->Play();
	}
	else
	{
		if (ImGui::Button("STOP", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
			App->Stop();
	}

	ImGui::SameLine();

	if (state == Application::play)
	{
		if (ImGui::Button("PAUSE", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
			App->Pause();
	}
	else if(state == Application::pause)
	{
		if (ImGui::Button("CONTINUE", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
			App->UnPause();
	}

	ImGui::SameLine(150.f);
	ImGui::Checkbox("Grid", &App->renderer3D->draw_plane);
	ImGui::SameLine();
	ImGui::Checkbox("Dbg Draw", &App->renderer3D->debug_draw);

    ImGui::End();
}
