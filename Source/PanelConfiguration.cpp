#include "PanelConfiguration.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "Module.h"
#include "ModuleFileSystem.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModulePhysics3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "ModuleEditor.h"

using namespace std;

// ---------------------------------------------------------
PanelConfiguration::PanelConfiguration() : Panel("Configuration", SDL_SCANCODE_F4),
	fps_log(FPS_LOG_SIZE),
	ms_log(FPS_LOG_SIZE)
{}

// ---------------------------------------------------------
PanelConfiguration::~PanelConfiguration()
{}

// ---------------------------------------------------------
void PanelConfiguration::Draw()
{
    ImGui::Begin("Configuration", &active, ImGuiWindowFlags_NoFocusOnAppearing);

	DrawApplication();

	if (InitModuleDraw(App->window))
		DrawModuleWindow(App->window);

	if (InitModuleDraw(App->audio))
		DrawModuleAudio(App->audio);

	if (InitModuleDraw(App->fs))
		DrawModuleFileSystem(App->fs);

	if (InitModuleDraw(App->input))
		DrawModuleInput(App->input);

    ImGui::End();
}

bool PanelConfiguration::InitModuleDraw(Module* module)
{
	bool ret = false;

	if (ImGui::CollapsingHeader(module->GetName()))
	{
		bool active = module->IsActive();
		if(ImGui::Checkbox("Active", &active))
			module->SetActive(active);
		ret = true;
	}

	return ret;
}

void PanelConfiguration::DrawApplication()
{
	if (ImGui::CollapsingHeader("Application"))
	{
		ImGui::Text("Name:");
		ImGui::SameLine();
		ImGui::TextColored(IMGUI_YELLOW, App->GetAppName());

		ImGui::Text("Organization:");
		ImGui::SameLine();
		ImGui::TextColored(IMGUI_YELLOW, App->GetOrganizationName());

		int max_fps = App->GetFramerateLimit();
		if (ImGui::SliderInt("Max FPS", &max_fps, 10, 100))
			App->SetFramerateLimit(max_fps);

		ImGui::Text("Limit Framerate:");
		ImGui::SameLine();
		ImGui::TextColored(IMGUI_YELLOW, "%i", App->GetFramerateLimit());

		ImGui::PlotHistogram("##framerate", &fps_log[0], fps_log.size(), 0, "Framerate", 0.0f, 120.0f, ImVec2(310, 100));
		ImGui::PlotHistogram("##milliseconds", &ms_log[0], ms_log.size(), 0, "Milliseconds", 0.0f, 60.0f, ImVec2(310, 100));
	}
}

void PanelConfiguration::DrawModuleAudio(ModuleAudio * module)
{
	// General Volume
	float volume = module->GetVolume();
	if (ImGui::SliderFloat("General Volume", (float*)&volume, 0.0f, 1.0f))
		module->SetVolume(volume);

	// Music Volume
	float music_volume = module->GetMusicVolume();
	if (ImGui::SliderFloat("Music Volume", (float*)&music_volume, 0.0f, 1.0f))
		module->SetMusicVolume(music_volume);

	// FX Volume
	float fx_volume = module->GetFXVolume();
	if (ImGui::SliderFloat("FX Volume", (float*)&fx_volume, 0.0f, 1.0f))
		module->SetFXVolume(fx_volume);

}

void PanelConfiguration::DrawModuleFileSystem(ModuleFileSystem * module)
{
	ImGui::Text("Base Path:");
	ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);
	ImGui::TextWrapped(module->GetBasePath());
	ImGui::PopStyleColor();

	ImGui::Text("Read Paths:");
	ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);
	ImGui::TextWrapped(module->GetReadPaths());
	ImGui::PopStyleColor();

	ImGui::Text("Write Path:");
	ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);
	ImGui::TextWrapped(module->GetWritePath());
	ImGui::PopStyleColor();
}

void PanelConfiguration::DrawModuleInput(ModuleInput * module)
{
	iPoint mouse = module->GetMousePosition();
	ImGui::Text("Mouse Position:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i,%i", mouse.x, mouse.y);

	mouse = module->GetMouseMotion();
	ImGui::Text("Mouse Motion:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i,%i", mouse.x, mouse.y);

	int wheel = module->GetMouseWheel();
	ImGui::Text("Mouse Wheel:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i", wheel);

	ImGui::Separator();

	ImGui::BeginChild("Input Log");
    ImGui::TextUnformatted(input_buf.begin());
    if (need_scroll)
        ImGui::SetScrollHere(1.0f);
    need_scroll = false;
	ImGui::EndChild();
}

void PanelConfiguration::DrawModuleWindow(ModuleWindow * module)
{
	ImGui::Text("Window Size:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i,%i", App->window->GetWidth(), App->window->GetHeigth());

	bool fullscreen = App->window->IsFullscreen();
	bool resizable = App->window->IsResizable();
	bool borderless = App->window->IsBorderless();
	bool full_desktop = App->window->IsFullscreenDesktop();

	ImGui::Checkbox("Fullscreen", &fullscreen);
	ImGui::SameLine();
	ImGui::Checkbox("Resizable", &resizable);
	ImGui::Checkbox("Borderless", &borderless);
	ImGui::SameLine();
	ImGui::Checkbox("Full Desktop", &full_desktop);
}

void PanelConfiguration::AddInput(const char * entry)
{
	input_buf.append(entry);
	need_scroll = true;
}

void PanelConfiguration::AddFPS(float fps, float ms)
{
	static uint count = 0;

	if (count == FPS_LOG_SIZE)
	{
		for (uint i = 0; i < FPS_LOG_SIZE - 1; ++i)
		{
			fps_log[i] = fps_log[i + 1];
			ms_log[i] = ms_log[i + 1];
		}
	}
	else
		++count;

	fps_log[count-1] = fps;
	ms_log[count-1] = ms;
}
