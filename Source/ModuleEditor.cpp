#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "Config.h"

#include "OpenGL.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION

#include "Nuklear/nuklear.h"
#include "Nuklear/nuklear_sdl_gl3.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

ModuleEditor::ModuleEditor(bool start_enabled) : Module("Editor", start_enabled)
{
}

// Destructor
ModuleEditor::~ModuleEditor()
{
}

// Called before render is available
bool ModuleEditor::Init(Config* config)
{
	LOG("Init editor gui with nuklear lib");

	context = nk_sdl_init(App->window->window);

	// Load font ------
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);

	char* buffer = nullptr;
	uint size = App->fs->Load(config->GetString("Font", ""), &buffer);

	struct nk_font* font = nk_font_atlas_add_from_memory(atlas, buffer, size, 16, 0);
    nk_sdl_font_stash_end();
    //nk_style_load_all_cursors(context, atlas->cursors);
    nk_style_set_font(context, &font->handle);

	RELEASE(buffer);

    /* style.c */
    /*set_style(ctx, THEME_WHITE);*/
    /*set_style(ctx, THEME_RED);*/

	return true;
}

update_status ModuleEditor::Update(float dt)
{
	struct nk_panel panel;

    nk_begin(context, &panel, "Demo", nk_rect(200, 200, 210, 250),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE);

    nk_layout_row_static(context, 30, 80, 1);
	nk_label(context, "Hello World", NK_TEXT_LEFT);
	static int property = 20;
	nk_layout_row_dynamic(context, 25, 1);
	nk_property_int(context, "Compression:", 0, &property, 100, 10, 1);

	nk_end(context);
	
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Freeing editor gui");
					  
	nk_sdl_shutdown();

	return true;
}

void ModuleEditor::BeginInput()
{
	nk_input_begin(context);
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	nk_sdl_handle_event(event);
}

void ModuleEditor::EndInput()
{
	nk_input_end(context);
}

void ModuleEditor::Draw()
{
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
}

bool ModuleEditor::IsHovered()
{
	return nk_window_is_any_hovered(context) != 0;
}
