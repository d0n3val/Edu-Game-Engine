#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "OpenGL.h"
#include "Primitive.h"
#include "ComponentCamera.h"
#include "Config.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module("Renderer", start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

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

		//Use Vsync
		bool set_vsync = config->GetBool("Vertical Sync", false);
		vsync = !set_vsync; // force change
		SetVSync(set_vsync);

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

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

		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].position = float3::zero;
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		lights[0].Active(true);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		glShadeModel(GL_SMOOTH);		 // Enables Smooth Shading
	}

	// Projection matrix for
	OnResize(App->window->GetWidth(), App->window->GetHeight());
	
	App->camera->Look(float3::zero);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	Color c = active_camera->background;
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetOpenGLViewMatrix());

	// light 0 on cam pos
	lights[0].position = App->camera->GetPosition();

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

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
	PPlane p(0, 1, 0, 0);
	p.axis = true;
	p.Render();

	// TODO: need to find out who is messing with the colors so I do not need this
	glColor3f(1.f,1.f,1.f);

	App->level->Draw();
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

void ModuleRenderer3D::Save(Config * config) const
{
	config->AddBool("Vertical Sync", GetVSync());
}

void ModuleRenderer3D::Load(Config * config)
{
	SetVSync(config->GetBool("Vertical Sync", true));
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf((GLfloat*) App->camera->GetOpenGLProjectionMatrix());

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
