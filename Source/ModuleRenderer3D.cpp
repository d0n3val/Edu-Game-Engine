#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleRenderer.h"
#include "ModuleEditorCamera.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "OpenGL.h"
#include "Primitive.h"
#include "ComponentCamera.h"
#include "Event.h"
#include "Config.h"
#include "DebugDraw.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "glew32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module("Renderer", start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{
}

// Called before render is available
bool ModuleRenderer3D::Init(Config* config)
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->GetWindow());
	if(context == nullptr)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		LOG("Glew library could not init %s\n", glewGetErrorString(err));
		ret = false;
	}
	else
		LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	if(ret == true)
	{
		// get version info
		LOG("Vendor: %s", glGetString(GL_VENDOR));
		LOG("Renderer: %s", glGetString(GL_RENDERER));
		LOG("OpenGL version supported %s", glGetString(GL_VERSION));
		LOG("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

		//Use Vsync
		bool set_vsync = config->GetBool("Vertical Sync", false);
		vsync = !set_vsync; // force change
		SetVSync(set_vsync);

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Blend for transparency
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
	}

	Load(config);

	return ret;
}

bool ModuleRenderer3D::Start(Config * config)
{
	// Projection matrix for
	//OnResize(App->window->GetWidth(), App->window->GetHeight());

	return true;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	ComponentCamera* cam = (App->IsPlay()) ? active_camera : App->camera->GetDummy();

	// Adjust projection if needed
	if (cam->projection_changed == true)
	{
		RefreshProjection();
		cam->projection_changed = false;
	}

	Color c = cam->background;
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(cam->GetOpenGLViewMatrix());

	return UPDATE_CONTINUE;
}

// Update: debug camera
update_status ModuleRenderer3D::Update(float dt)
{
	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	// debug draw ---
	if (draw_plane == true)
	{
		PPlane p(0, 1, 0, 0);
		p.axis = true;
		p.Render();
	}

	// TODO: need to find out who is messing with the colors so I do not need this
	glColor3f(1.f,1.f,1.f);

	//App->level->Draw();
	App->renderer->Draw(active_camera, App->window->GetWidth(), App->window->GetHeight());

	if (debug_draw == true)
	{
		BeginDebugDraw();
		App->DebugDraw();
		EndDebugDraw();
	}

	App->editor->Draw();

	SDL_GL_SwapWindow(App->window->GetWindow());
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}

void ModuleRenderer3D::ReceiveEvent(const Event& event)
{
	switch (event.type)
	{
		case Event::window_resize:
			OnResize(event.point2d.x, event.point2d.y);
		break;
	}
}

void ModuleRenderer3D::Save(Config * config) const
{
	config->AddBool("Vertical Sync", GetVSync());
	config->AddBool("Debug Plane", draw_plane);
	config->AddBool("Debug Draw", debug_draw);
}

void ModuleRenderer3D::Load(Config * config)
{
	SetVSync(config->GetBool("Vertical Sync", true));
	draw_plane = config->GetBool("Debug Plane", true);
	debug_draw = config->GetBool("Debug Draw", true);
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	ComponentCamera* cam = (App->IsPlay()) ? active_camera : App->camera->GetDummy();

	cam->SetAspectRatio((float)width / (float)height);
	glViewport(0, 0, width, height);

	RefreshProjection();
}

void ModuleRenderer3D::RefreshProjection()
{
	ComponentCamera* cam = (App->IsPlay()) ? active_camera : App->camera->GetDummy();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf((GLfloat*) cam->GetOpenGLProjectionMatrix());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool ModuleRenderer3D::GetVSync() const
{
	return vsync;
}

void ModuleRenderer3D::SetVSync(bool vsync)
{
	if (this->vsync != vsync)
	{
		this->vsync = vsync;
		if(SDL_GL_SetSwapInterval(vsync ? 1 : 0) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}
}

const char * ModuleRenderer3D::GetDriver() const
{
	return SDL_GetCurrentVideoDriver();
}
