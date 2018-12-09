#include "PanelConfiguration.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "Module.h"
#include "ModuleHardware.h"
#include "ModuleFileSystem.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModulePhysics3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ModuleSceneLoader.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModulePhysics3D.h"
#include "ModuleLevelManager.h"
#include "ResourceTexture.h"
#include "PanelProperties.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "mmgr/mmgr.h"

using namespace std;

// ---------------------------------------------------------
PanelConfiguration::PanelConfiguration() : Panel("Configuration"),
	fps_log(FPS_LOG_SIZE), ms_log(FPS_LOG_SIZE)
{
	width = default_width;
	height = default_height;
	posx = default_posx;
	posy = default_posy;
}

// ---------------------------------------------------------
PanelConfiguration::~PanelConfiguration()
{}

// ---------------------------------------------------------
void PanelConfiguration::Draw()
{
    ImGui::Begin("Configuration", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);

	if (ImGui::BeginMenu("Options"))
	{
		ImGui::MenuItem("Set Defaults");
		if (ImGui::MenuItem("Load"))
			App->LoadPrefs();

		if (ImGui::MenuItem("Save"))
			App->SavePrefs();

		ImGui::EndMenu();
	}

	DrawApplication();

	if (InitModuleDraw(App->level))
		DrawModuleLevel(App->level);

	if (InitModuleDraw(App->window))
		DrawModuleWindow(App->window);

	if (InitModuleDraw(App->renderer3D))
		DrawModuleRenderer(App->renderer3D);

	if (InitModuleDraw(App->camera))
		DrawModuleCamera(App->camera);

	if (InitModuleDraw(App->tex))
		DrawModuleTextures(App->tex);

	if (InitModuleDraw(App->audio))
		DrawModuleAudio(App->audio);

	if (InitModuleDraw(App->physics3D))
		DrawModulePhysics(App->physics3D);

	if (InitModuleDraw(App->fs))
		DrawModuleFileSystem(App->fs);

	if (InitModuleDraw(App->input))
		DrawModuleInput(App->input);

	if (InitModuleDraw(App->hw))
		DrawModuleHardware(App->hw);

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
		static char app_name[120];
		strcpy_s(app_name, 120, App->GetAppName());
		if (ImGui::InputText("App Name", app_name, 120, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			App->SetAppName(app_name);

		static char org_name[120];
		strcpy_s(org_name, 120, App->GetOrganizationName());
		if (ImGui::InputText("Organization", org_name, 120, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			App->SetOrganizationName(org_name);

		int max_fps = App->GetFramerateLimit();
		if (ImGui::SliderInt("Max FPS", &max_fps, 0, 120))
			App->SetFramerateLimit(max_fps);

		ImGui::Text("Limit Framerate:");
		ImGui::SameLine();
		ImGui::TextColored(IMGUI_YELLOW, "%i", App->GetFramerateLimit());

		char title[25];
		sprintf_s(title, 25, "Framerate %.1f", fps_log[fps_log.size()-1]);
		ImGui::PlotHistogram("##framerate", &fps_log[0], fps_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
		sprintf_s(title, 25, "Milliseconds %0.1f", ms_log[ms_log.size()-1]);
		ImGui::PlotHistogram("##milliseconds", &ms_log[0], ms_log.size(), 0, title, 0.0f, 40.0f, ImVec2(310, 100));

		// Memory --------------------
		sMStats stats = m_getMemoryStatistics();
		static int speed = 0;
		static vector<float> memory(100);
		if (++speed > 20)
		{
			speed = 0;
			if (memory.size() == 100)
			{
				for (uint i = 0; i < 100 - 1; ++i)
					memory[i] = memory[i + 1];

				memory[100 - 1] = (float)stats.totalReportedMemory;
			}
			else
				memory.push_back((float)stats.totalReportedMemory);
		}

		ImGui::PlotHistogram("##memory", &memory[0], memory.size(), 0, "Memory Consumption", 0.0f, (float)stats.peakReportedMemory * 1.2f, ImVec2(310,100));

		ImGui::Text("Total Reported Mem: %u",  stats.totalReportedMemory);
		ImGui::Text("Total Actual Mem: %u", stats.totalActualMemory);
		ImGui::Text("Peak Reported Mem: %u", stats.peakReportedMemory);
		ImGui::Text("Peak Actual Mem: %u", stats.peakActualMemory);
		ImGui::Text("Accumulated Reported Mem: %u", stats.accumulatedReportedMemory);
		ImGui::Text("Accumulated Actual Mem: %u", stats.accumulatedActualMemory);
		ImGui::Text("Accumulated Alloc Unit Count: %u", stats.accumulatedAllocUnitCount);
		ImGui::Text("Total Alloc Unit Count: %u", stats.totalAllocUnitCount);
		ImGui::Text("Peak Alloc Unit Count: %u", stats.peakAllocUnitCount);
	}
}

void PanelConfiguration::DrawModuleHardware(ModuleHardware * module)
{
	ModuleHardware::hw_info info = module->GetInfo();
	IMGUI_PRINT("SDL Version:", info.sdl_version);

	ImGui::Separator();
	IMGUI_PRINT("CPUs:", "%u (Cache: %ukb)", info.cpu_count, info.l1_cachekb);
	IMGUI_PRINT("System RAM:", "%.1fGb", info.ram_gb);
	IMGUI_PRINT("Caps:", "%s%s%s%s%s%s",
		info.rdtsc ? "RDTSC," : "",
		info.altivec ? "AltiVec," : "",
		info.mmx ? "MMX," : "",
		info.now3d ? "3DNow," : "",
		info.sse ? "SSE," : "",
		info.sse2 ? "SSE2," : "");
	IMGUI_PRINT("", "%s%s%s%s%s",
		info.sse3 ? "SSE3," : "",
		info.sse41 ? "SSE41," : "",
		info.sse42 ? "SSE42," : "",
		info.avx ? "AVX," : "",
		info.avx2 ? "AVX2" : "" );


	ImGui::Separator();
	IMGUI_PRINT("GPU:", "vendor %u device %u", info.gpu_vendor, info.gpu_device);
	IMGUI_PRINT("Brand:", info.gpu_brand);
	IMGUI_PRINT("VRAM Budget:", "%.1f Mb", info.vram_mb_budget);
	IMGUI_PRINT("VRAM Usage:", "%.1f Mb", info.vram_mb_usage);
	IMGUI_PRINT("VRAM Available:", "%.1f Mb", info.vram_mb_available);
	IMGUI_PRINT("VRAM Reserved:", "%.1f Mb", info.vram_mb_reserved);
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
	int mouse_x, mouse_y;
	module->GetMousePosition(mouse_x, mouse_y);
	ImGui::Text("Mouse Position:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i,%i", mouse_x, mouse_y);

	module->GetMouseMotion(mouse_x, mouse_y);
	ImGui::Text("Mouse Motion:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%i,%i", mouse_x, mouse_y);

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
	static bool waiting_to_load_icon = false;

	if (waiting_to_load_icon == true && App->editor->FileDialog("bmp"))
	{
		const char* file = App->editor->CloseFileDialog();
		if(file != nullptr)
			App->window->SetIcon(file);
		waiting_to_load_icon = false;
	}

	ImGui::Text("Icon: ");
	ImGui::SameLine();
	if (ImGui::Selectable(App->window->GetIcon()))
		waiting_to_load_icon = true;

	float brightness = App->window->GetBrightness();
	if (ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f))
		App->window->SetBrightness(brightness);

	uint w, h, min_w, min_h, max_w, max_h;
	App->window->GetMaxMinSize(min_w, min_h, max_w, max_h);
	w = App->window->GetWidth();
	h = App->window->GetHeight();

	if (ImGui::SliderInt("Width", (int*)&w, min_w, max_w))
		App->window->SetWidth(w);

	if (ImGui::SliderInt("Height", (int*)&h, min_h, max_h))
		App->window->SetHeigth(h);

	ImGui::Text("Refresh rate:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%u", App->window->GetRefreshRate());

	bool fullscreen = App->window->IsFullscreen();
	bool resizable = App->window->IsResizable();
	bool borderless = App->window->IsBorderless();
	bool full_desktop = App->window->IsFullscreenDesktop();

	if (ImGui::Checkbox("Fullscreen", &fullscreen))
		App->window->SetFullscreen(fullscreen);

	ImGui::SameLine();
	if (ImGui::Checkbox("Resizable", &resizable))
		App->window->SetResizable(resizable);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Restart to apply");

	if (ImGui::Checkbox("Borderless", &borderless))
		App->window->SetBorderless(borderless);

	ImGui::SameLine();
	if (ImGui::Checkbox("Full Desktop", &full_desktop))
		App->window->SetFullScreenDesktop(full_desktop);
}

void PanelConfiguration::DrawModuleRenderer(ModuleRenderer3D * module)
{
	ImGui::Text("Driver:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, App->renderer3D->GetDriver());

	bool vsync = App->renderer3D->GetVSync();
	if (ImGui::Checkbox("Vertical Sync", &vsync))
		App->renderer3D->SetVSync(vsync);
}

void PanelConfiguration::DrawModuleTextures(ModuleTextures * module)
{
	int i = 0;
	int cols = 5;
	vector<const Resource*> loaded_textures;
	App->resources->GatherResourceType(loaded_textures, Resource::texture);

	for (vector<const Resource*>::const_iterator it = loaded_textures.begin(); it != loaded_textures.end(); ++it)
	{
		const ResourceTexture* info = (const ResourceTexture*) (*it);

		ImGui::Image((ImTextureID) info->GetID(), ImVec2(50.f, 50.f), ImVec2(0,1), ImVec2(1,0));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(IMGUI_YELLOW, info->GetFile());
			ImGui::Text("%s", (info->GetID() != 0) ? "Loaded in VRAM" : "Not in VRAM");
			ImGui::Text("(%u,%u) %0.1f Mb %s", info->GetWidth(), info->GetHeight(), info->GetBytes() / (1024.f*1024.f) , info->GetFormatStr());
			ImGui::Text("Depth: %u Bpp: %u", info->GetDepth(), info->GetBPP());

			ImVec2 size((float)info->GetWidth(), (float)info->GetHeight());
			float max_size = 250.f;

			if (size.x > max_size || size.y > max_size)
			{
				if (size.x > size.y)
				{
					size.y *= max_size / size.x;
					size.x = max_size;
				}
				else
				{
					size.x *= max_size / size.y;
					size.y = max_size;
				}
			}

			ImGui::Image((ImTextureID) info->GetID(), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));
			ImGui::EndTooltip();
		}
        if ((i++ % 5) < 4) ImGui::SameLine();
	}
	ImGui::NewLine();
}

void PanelConfiguration::DrawModuleCamera(ModuleEditorCamera * module)
{
	ImGui::DragFloat3("Front", &App->camera->GetDummy()->frustum.front.x, 0.1f);
	ImGui::DragFloat3("Up", &App->camera->GetDummy()->frustum.up.x, 0.1f);
	ImGui::DragFloat3("Position", &App->camera->GetDummy()->frustum.pos.x, 0.1f);
	ImGui::DragFloat("Mov Speed", &App->camera->mov_speed, 0.1f, 0.1f);
	ImGui::DragFloat("Rot Speed", &App->camera->rot_speed, 0.05f, 0.01f);
	ImGui::DragFloat("Zoom Speed", &App->camera->zoom_speed, 0.1f, 0.1f);
	App->editor->props->DrawCameraComponent(module->GetDummy());
}

void PanelConfiguration::DrawModulePhysics(ModulePhysics3D * module)
{
	ImGui::Checkbox("Paused", &App->physics3D->paused);

	float3 gravity = App->physics3D->GetGravity();
	if (ImGui::DragFloat3("Gravity", &gravity.x, 0.1f))
		App->physics3D->SetGravity(gravity);

	ImGui::Checkbox("Debug Draw", &App->physics3D->debug);

	ImGui::Text("Debug Draw Flags:");
	uint mode = App->physics3D->GetDebugMode();
/*
		DBG_NoDebug=0,
		DBG_DrawWireframe = 1,
		DBG_DrawAabb=2,
		DBG_DrawFeaturesText=4,
		DBG_DrawContactPoints=8,
		DBG_NoDeactivation=16,
		DBG_NoHelpText = 32,
		DBG_DrawText=64,
		DBG_ProfileTimings = 128,
		DBG_EnableSatComparison = 256,
		DBG_DisableBulletLCP = 512,
		DBG_EnableCCD = 1024,
		DBG_DrawConstraints = (1 << 11),
		DBG_DrawConstraintLimits = (1 << 12),
		DBG_FastWireframe = (1<<13),
		DBG_DrawNormals = (1<<14),
		DBG_DrawFrames = (1<<15),
*/
	if (ImGui::CheckboxFlags("Wire Frame", &mode, 1))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("AABB", &mode, 2))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Contact Points", &mode, 8))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("No Deactivation", &mode, 16))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Constraints", &mode, 1<<11))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Constraints Limits", &mode, 1<<12))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Fast Wires", &mode, 1<<13))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Normals", &mode, 1<<14))
		App->physics3D->SetDebugMode(mode);

	if (ImGui::CheckboxFlags("Frames", &mode, 1<<15))
		App->physics3D->SetDebugMode(mode);
}

float rnd(float min, float max)
{
	return max * ((float) rand() / (float) RAND_MAX);
}



void PanelConfiguration::DrawModuleLevel(ModuleLevelManager * module)
{
	ImGui::Checkbox("Debug Draw Quadtree", &module->draw_quadtree);
	if (ImGui::Button("Add random box to Quadtree"))
	{
		// this will leak memory, but it is only for visualization purposes
		float3 max = module->quadtree.root->box.Size();

		GameObject* dummy = new GameObject(nullptr, "test for quadtree");
		float3 pos;
		pos.Set(rnd(0.f, max.x), rnd(0.f, max.y), rnd(0.f, max.z));
		float size = rnd(1.f, max.x * 0.25f);
		dummy->global_bbox.SetFrom(Sphere(pos, size));
		module->quadtree.Insert(dummy);
	}

	ImGui::Separator();
	RecursiveDrawQuadtree(module->quadtree.root);

	if (ImGui::CollapsingHeader("Quadtree Minimap"))
	{
		// Draw quadtree like a minimap
		ImGui::Separator();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(50, 50, 50), ImColor(50, 50, 60), ImColor(60, 60, 70), ImColor(50, 50, 60));
		draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255, 255, 255));
		ImGui::InvisibleButton("Canvas", canvas_size);

		float quad_posx = module->quadtree.root->box.minPoint.x;
		float quad_posz = module->quadtree.root->box.minPoint.z;
		float quad_width = module->quadtree.root->box.maxPoint.x - quad_posx;
		float quad_height = module->quadtree.root->box.maxPoint.z - quad_posz;

		ImVec2 up_left;
		ImVec2 bottom_right;

		vector<GameObject*> objects;
		module->quadtree.CollectObjects(objects);
		for (vector<GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
		{
			AABB box = (*it)->global_bbox.MinimalEnclosingAABB();

			up_left.x = ((box.minPoint.x - quad_posx) * (float)canvas_size.x) / quad_width;
			up_left.y = ((box.minPoint.z - quad_posz) * (float)canvas_size.y) / quad_height;

			bottom_right.x = ((box.maxPoint.x - quad_posx) * (float)canvas_size.x) / quad_width;
			bottom_right.y = ((box.maxPoint.z - quad_posz) * (float)canvas_size.y) / quad_height;

			ImColor color(200, 0, 0, 100);

			if ((*it)->visible == false)
				color = ImColor(0, 0, 100, 100);

			draw_list->AddRectFilled(
				ImVec2(canvas_pos.x + up_left.x, canvas_pos.y + up_left.y),
				ImVec2(canvas_pos.x + bottom_right.x, canvas_pos.y + bottom_right.y),
				color, 0.0f, 10);

			draw_list->AddRect(
				ImVec2(canvas_pos.x + up_left.x, canvas_pos.y + up_left.y),
				ImVec2(canvas_pos.x + bottom_right.x, canvas_pos.y + bottom_right.y),
				ImColor(255, 255, 255), 0.0f, 10, 0.5f);
		}

		vector<const QuadtreeNode*> nodes;
		module->quadtree.CollectBoxes(nodes);
		for (vector<const QuadtreeNode*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			AABB box = (*it)->box;

			up_left.x = ((box.minPoint.x - quad_posx) * (float)canvas_size.x) / quad_width;
			up_left.y = ((box.minPoint.z - quad_posz) * (float)canvas_size.y) / quad_height;

			bottom_right.x = ((box.maxPoint.x - quad_posx) * (float)canvas_size.x) / quad_width;
			bottom_right.y = ((box.maxPoint.z - quad_posz) * (float)canvas_size.y) / quad_height;

			draw_list->AddRect(
				ImVec2(canvas_pos.x + up_left.x, canvas_pos.y + up_left.y),
				ImVec2(canvas_pos.x + bottom_right.x, canvas_pos.y + bottom_right.y),
				ImColor(255, 255, 0), 0.0f, 10, 0.1f);
		}
	}
}

void PanelConfiguration::RecursiveDrawQuadtree(QuadtreeNode * node)
{
	uint flags = 0;

	if (node->childs[0] == nullptr)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx(node, flags, "QNode"))
	{
		for (list<GameObject*>::const_iterator it = node->objects.begin(); it != node->objects.end(); ++it)
			ImGui::Text("%s", (*it)->name.c_str());

		for (uint i = 0; i < 4; ++i)
			if (node->childs[i] != nullptr)
				RecursiveDrawQuadtree(node->childs[i]);

		ImGui::TreePop();
	}

}

void PanelConfiguration::AddInput(const char * entry)
{
	input_buf.appendf(entry);
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
