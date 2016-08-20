#include "Globals.h"
#include "Application.h"
#include "ModuleEditorCamera.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
// delete --
#include "ModuleLevelManager.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"

ModuleEditorCamera::ModuleEditorCamera(bool start_enabled) : Module("Camera", start_enabled)
{
	dummy = new ComponentCamera(nullptr);
}

ModuleEditorCamera::~ModuleEditorCamera()
{
	RELEASE(dummy);
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::Init(Config* config)
{
	dummy->OnLoad(config);
	App->renderer3D->active_camera = dummy;

	return true;
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::Start(Config* config)
{
	LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::CleanUp()
{
	LOG("Cleaning camera");

	App->renderer3D->active_camera = nullptr;
	return true;
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Save(Config * config) const
{
	if(config != nullptr)
		dummy->OnSave(*config);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Load(Config * config)
{
	dummy->OnLoad(config);
}

// -----------------------------------------------------------------
update_status ModuleEditorCamera::Update(float dt)
{
	Frustum* frustum = &dummy->frustum;
	// OnKeys WASD keys -----------------------------------
	if (App->editor->UsingKeyboard() == false)
	{
		float speed = 15.0f;

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) speed *= 5.0f;
		if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) speed *= 0.5f;

		float3 right(frustum->WorldRight());
		float3 forward(frustum->front);

		float3 movement(float3::zero);

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) movement += forward;
		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) movement -= forward;
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) movement -= right;
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) movement += right;
		if (App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT) movement += float3::unitY;
		if (App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) movement -= float3::unitY;

		if (movement.Equals(float3::zero) == false)
		{
			frustum->Translate(movement * (speed * dt));
			looking = false;
		}
	}

	// Mouse motion ----------------
	if (App->editor->UsingMouse() == false)
	{
		int motion_x, motion_y;
		App->input->GetMouseMotion(motion_x, motion_y);
		if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT && (motion_x != 0 || motion_y != 0))
		{
			float dx = (float)-motion_x;
			float dy = (float)-motion_y;

			if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
			{
				// rotate around a position - lookat
				float sensitivity = 0.01f;
				dx *= sensitivity;
				dy *= sensitivity;

				float3 point = looking_at;

				// fake point should be a ray colliding with something
				if (looking == false)
					point = frustum->pos + frustum->front * 50.0f;

				float3 focus = frustum->pos - point;

				Quat qy(frustum->up, dx);
				Quat qx(frustum->WorldRight(), dy);

				focus = qx.Transform(focus);
				focus = qy.Transform(focus);

				frustum->pos = focus + point;

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
					frustum->front = q.Mul(frustum->front).Normalized();
					// would not need this is we were rotating in the local Y, but that is too disorienting
					frustum->up = q.Mul(frustum->up).Normalized();
				}

				// y motion makes the camera rotate in X local axis 
				if (dy != 0.f)
				{
					Quat q = Quat::RotateAxisAngle(frustum->WorldRight(), dy);

					frustum->up = q.Mul(frustum->up).Normalized();
					frustum->front = q.Mul(frustum->front).Normalized();
				}
			}

		}
		// Mouse wheel
		int wheel = App->input->GetMouseWheel();
		if (wheel != 0)
		{
			float speed = 3.0f;
			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) speed *= 5.0f;
			float3 p = frustum->front * ((float)wheel * speed);
			frustum->pos += p;
		}
	}

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
float3 ModuleEditorCamera::GetPosition() const
{
	return dummy->frustum.pos;
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Look(const float3& position)
{
	dummy->Look(position);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::CenterOn(const float3& position, float distance)
{
	float3 v = dummy->frustum.front.Neg();
	dummy->frustum.pos = position + (v * distance);
	looking_at = position;
	looking = true;
}

// -----------------------------------------------------------------
ComponentCamera * ModuleEditorCamera::GetDummy() const
{
	return dummy;
}
