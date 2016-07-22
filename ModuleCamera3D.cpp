#include "Globals.h"
#include "Application.h"
#include "PhysBody3D.h"
#include "ModuleCamera3D.h"

ModuleCamera3D::ModuleCamera3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	CalculateViewMatrix();

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 0.0f, 5.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);

	following = NULL;
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");

	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	// Follow code
	if(following != NULL)
	{
		mat4x4 m;
		following->GetTransform(&m);

		Look(Position, m.translation(), true);

		// Correct height
		Position.y = (15.0*Position.y + Position.y + following_height) / 16.0;

		// Correct distance
		vec3 cam_to_target = m.translation() - Position;
		float dist = length(cam_to_target);
		float correctionFactor = 0.f;
		if(dist < min_following_dist)
		{
			correctionFactor = 0.15*(min_following_dist - dist) / dist;
		}
		if(dist > max_following_dist)
		{
			correctionFactor = 0.15*(max_following_dist - dist) / dist;
		}
		Position -= correctionFactor * cam_to_target;
	}

	// Implement a debug camera with keys and mouse

	// OnKeys WASD keys -----------------------------------
	float Speed = 5.0f;

	if(App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) Speed *= 2.0f;
	if(App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) Speed *= 0.5f;

	float Distance = Speed * dt;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	if(App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) Movement += Forward;
	if(App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) Movement -= Forward;
	if(App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) Movement -= Right;
	if(App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) Movement += Right;
	if(App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT) Movement += Up;
	if(App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) Movement -= Up;

	Position += Movement;
	Reference += Movement;
	
	// Mouse motion ----------------

	if(App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{

		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();

		float Sensitivity = 0.25f;

		Position -= Reference;

		if(dx != 0)
		{
			float DeltaX = (float)dx * Sensitivity;

			X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
			Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
			Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
		}

		if(dy != 0)
		{
			float DeltaY = (float)dy * Sensitivity;

			Y = rotate(Y, DeltaY, X);
			Z = rotate(Z, DeltaY, X);

			if(Y.y < 0.0f)
			{
				Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				Y = cross(Z, X);
			}
		}

		Position = Reference + Z * length(Position);
	}

	// Mouse wheel -----------------------

	float zDelta = (float) App->input->GetMouseZ();

	Position -= Reference;

	if(zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if(zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;

	// Recalculate matrix -------------
	CalculateViewMatrix();

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
void ModuleCamera3D::Move(const vec3 &Movement)
{
	Position += Movement;
	Reference += Movement;

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
float* ModuleCamera3D::GetViewMatrix()
{
	return &ViewMatrix;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
	ViewMatrixInverse = inverse(ViewMatrix);
}

// -----------------------------------------------------------------
void ModuleCamera3D::Follow(PhysBody3D* body, float min, float max, float height)
{
	min_following_dist = min;
	max_following_dist = max;
	following_height = height;
	following = body;
}

// -----------------------------------------------------------------
void ModuleCamera3D::UnFollow()
{
	following = NULL;
}