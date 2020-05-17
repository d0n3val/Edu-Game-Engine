#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleRenderer.h"
#include "ModuleEditorCamera.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "ModuleDebugDraw.h"
#include "ModuleHints.h"
#include "OpenGL.h"
#include "Primitive.h"
#include "ComponentCamera.h"
#include "Event.h"
#include "Config.h"
#include "DebugDraw.h"
#include "Viewport.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "glew32.lib") /* link Microsoft OpenGL lib   */

void __stdcall DebugMessageGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (id == 131185 || id == 131204) return;

	char tmp_string[4096];

	const char* tmp_source = "";

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             tmp_source = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   tmp_source = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: tmp_source = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     tmp_source = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     tmp_source = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           tmp_source = "Other"; break;
	};

	const char* tmp_type = "";

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               tmp_type = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: tmp_type = "Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  tmp_type = "Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         tmp_type = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         tmp_type = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              tmp_type = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          tmp_type = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           tmp_type = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               tmp_type = "Other"; break;
	};


	const char* tmp_severity = "";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         tmp_severity = "high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       tmp_severity = "medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          tmp_severity = "low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: tmp_severity = "notification"; break;
	};

	sprintf_s(tmp_string, 4095, "<Source:%s> <Type:%s> <Severity:%s> <ID:%d> <Message:%s>\n", tmp_source, tmp_type, tmp_severity, id, message);
	OutputDebugString(tmp_string);
}


ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module("Renderer", start_enabled), viewport(0)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{
    delete viewport;
}

// Called before render is available
bool ModuleRenderer3D::Init(Config* config)
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //SDL_GL_CONTEXT_PROFILE_CORE);


	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

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
		
        //glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glClearDepth(1.0f);
		glClearStencil(0);
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		
		//Initialize clear color
		glClearColor(0.4f, 0.4f, 0.4f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Blend for transparency
        glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);

        glEnable(GL_MULTISAMPLE);  
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);  
    }

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	glDebugMessageCallback(&DebugMessageGL, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);


    viewport = new Viewport;

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

	Color c = cam->background;
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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
    viewport->Draw(active_camera);
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
    viewport->Save(config);
}

void ModuleRenderer3D::Load(Config * config)
{
	SetVSync(config->GetBool("Vertical Sync", true));
    viewport->Load(config);
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	ComponentCamera* cam = (App->IsPlay()) ? active_camera : App->camera->GetDummy();

	cam->SetAspectRatio((float)width / (float)height);
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
