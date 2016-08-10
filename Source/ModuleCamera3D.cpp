#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
// delete --
#include "ModuleLevelManager.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"

ModuleCamera3D::ModuleCamera3D(bool start_enabled) : Module("Camera", start_enabled)
{
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Init(Config* config)
{
	float aspect_ratio = (float) App->window->GetWidth() / (float) App->window->GetHeight();
	float fov = 60.f; // degrees

	frustum.type = FrustumType::PerspectiveFrustum;
	frustum.pos = float3(-10.f, 5.0f, 3.0f);
	frustum.front = float3(0.f, 0.f, 1.f);
	frustum.up = float3(0.f, 1.f, 0.f);
	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 500.0f;
	frustum.verticalFov = DEGTORAD * fov;
	// More about FOV: http://twgljs.org/examples/fov-checker.html
	// fieldOfViewX = 2 * atan(tan(fieldOfViewY * 0.5) * aspect)
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * aspect_ratio);


	return true;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start(Config* config)
{
	LOG("Setting up the camera");
	bool ret = true;

	GameObject* go = App->level->CreateGameObject(nullptr, float3::zero, float3::one, Quat::identity, "Test Camera");
	ComponentCamera* c = (ComponentCamera*) go->CreateComponent(ComponentTypes::Camera);

	App->renderer3D->active_camera = c;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");

	App->renderer3D->active_camera = nullptr;
	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	// Ignore camera movement if we are using the editor
	if (App->editor->UsingInput() == true)
		return UPDATE_CONTINUE;

	// OnKeys WASD keys -----------------------------------
	float speed = 15.0f;

	if(App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) speed *= 5.0f;
	if(App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) speed *= 0.5f;

	float3 right(frustum.WorldRight());
	float3 forward(frustum.front);

	float3 movement(float3::zero);

	if(App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) movement += forward;
	if(App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) movement -= forward;
	if(App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) movement -= right;
	if(App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) movement += right;
	if(App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT) movement += float3::unitY;
	if(App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) movement -= float3::unitY;

	if (movement.Equals(float3::zero) == false)
	{
		frustum.Translate(movement * (speed * dt));
		looking = false;
	}

	// Mouse motion ----------------
	iPoint motion = App->input->GetMouseMotion();
	if(App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT && (motion.x != 0 || motion.y != 0))
	{
		float dx = (float)-motion.x;
		float dy = (float)-motion.y;

		if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
		{
			// rotate around a position - lookat
			float sensitivity = 0.01f;
			dx *= sensitivity;
			dy *= sensitivity;

			float3 point = looking_at;

			// fake point should be a ray colliding with something
			if (looking == false)
				point = frustum.pos + frustum.front * 50.0f;

			float3 focus = frustum.pos - point;

			Quat qy(frustum.up, dx);
			Quat qx(frustum.WorldRight(), dy);

			focus = qx.Transform(focus);
			focus = qy.Transform(focus);

			frustum.pos = focus + point;

			Look(point);
		}
		else
		{
			// WASD style lookat
			looking = false;
			float sensitivity = 0.01f;
			dx *= sensitivity;
			dy *= sensitivity;

			// x motion make the camera rotate in Y absolute axis (0,1,0) (not local)
			if (dx != 0.f)
			{
				Quat q = Quat::RotateY(dx);
				frustum.front = q.Mul(frustum.front).Normalized();
				// would not need this is we were rotating in the local Y, but that is too disorienting
				frustum.up = q.Mul(frustum.up).Normalized();
			}

			// y motion makes the camera rotate in X local axis 
			if (dy != 0.f)
			{
				Quat q = Quat::RotateAxisAngle(frustum.WorldRight(), dy);

				frustum.up = q.Mul(frustum.up).Normalized();
				frustum.front = q.Mul(frustum.front).Normalized();
			}
		}

		// Mouse wheel
		int wheel = App->input->GetMouseWheel();
		if (wheel != 0)
		{
			float sensitivity = 1.0f;
			float3 p = frustum.front * ((float)wheel * sensitivity);
			frustum.pos += p;
		}
	}

	return UPDATE_CONTINUE;
}

float3 ModuleCamera3D::GetPosition() const
{
	return frustum.pos;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const float3& position)
{
	float3 dir = position - frustum.pos;

	float3x3 m = float3x3::LookAt(frustum.front, dir.Normalized(), frustum.up, float3::unitY);

	frustum.front = m.MulDir(frustum.front).Normalized();
	frustum.up = m.MulDir(frustum.up).Normalized();
}

// -----------------------------------------------------------------
float * ModuleCamera3D::GetOpenGLViewMatrix()
{
	static float4x4 m;
	
	m = frustum.ViewMatrix();
	m.Transpose();

	return (float*) m.v;
}

// -----------------------------------------------------------------
float * ModuleCamera3D::GetOpenGLProjectionMatrix()
{
	static float4x4 m;
	
	m = frustum.ProjectionMatrix();
	m.Transpose();

	return (float*) m.v;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CenterOn(const float3& position, float distance)
{
	float3 v = frustum.front.Neg();
	frustum.pos = position + (v * distance);
	looking_at = position;
	looking = true;
}
