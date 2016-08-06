#include "Globals.h"
#include "Application.h"
#include "PhysBody3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"

ModuleCamera3D::ModuleCamera3D(bool start_enabled) : Module("Camera", start_enabled)
{
	CalculateViewMatrix();

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 0.0f, 5.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
	reference = float3::zero;
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Init(Config* config)
{
	// More about FOV: http://twgljs.org/examples/fov-checker.html

	float aspect_ratio = (float) App->window->GetWidth() / (float) App->window->GetHeigth();
	float fov = 60.f; // degrees

	frustum.type = FrustumType::OrthographicFrustum;
	frustum.pos = float3(0.f, 0.0f, 0.0f);
	frustum.front = float3(0.f, 0.f, 1.f);
	frustum.up = float3(0.f, 1.f, 0.f);
	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 5000.0f;
	frustum.verticalFov = DEGTORAD * fov;
	// fieldOfViewX = 2 * atan(tan(fieldOfViewY * 0.5) * aspect)
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * aspect_ratio);

	LOG("FOV X %0.1f FOV Y %0.1f", frustum.horizontalFov *RADTODEG, frustum.verticalFov * RADTODEG);
	LOG("Aspect ratio: %0.3f vs %0.3f vs 0.3f", aspect_ratio, frustum.AspectRatio(), frustum.verticalFov / frustum.horizontalFov);

	return true;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start(Config* config)
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
	if(following != nullptr)
	{
		mat4x4 m;
		following->GetTransform(&m);

		Look(Position, m.translation(), true);

		// Correct height
		Position.y = (15.0f*Position.y + Position.y + following_height) / 16.0f;

		// Correct distance
		vec3 cam_to_target = m.translation() - Position;
		float dist = length(cam_to_target);
		float correctionFactor = 0.f;
		if(dist < min_following_dist)
		{
			correctionFactor = 0.15f*(min_following_dist - dist) / dist;
		}
		if(dist > max_following_dist)
		{
			correctionFactor = 0.15f*(max_following_dist - dist) / dist;
		}
		Position -= correctionFactor * cam_to_target;
	}

	// Implement a debug camera with keys and mouse
	if (App->editor->UsingInput() == true)
		return UPDATE_CONTINUE;

	// OnKeys WASD keys -----------------------------------
	float Speed = 5.0f;

	if(App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) Speed *= 5.0f;
	if(App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) Speed *= 0.5f;

	float Distance = Speed * dt;

	float3 Up(0.f, 1.f, 0.f);
	float3 Right(frustum.WorldRight());
	float3 Forward(frustum.front);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	float3 Movement(float3::zero);

	if(App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) Movement += Forward;
	if(App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) Movement -= Forward;
	if(App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) Movement -= Right;
	if(App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) Movement += Right;
	if(App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT) Movement += Up;
	if(App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) Movement -= Up;

	frustum.pos += Movement;
	//Position += Movement;
	//Reference += Movement;
	
	// Mouse motion ----------------

	if(App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		iPoint motion = App->input->GetMouseMotion();
		float sensitivity = 0.01f;
		float dx = (float)-motion.x * sensitivity;
		float dy = (float)-motion.y * sensitivity;

		if(dx != 0.f)
			frustum.Transform(Quat(float3::unitY, dx));

		if (dy != 0.f) {
			//frustum.Transform(Quat(frustum.WorldRight(), dy));
			LOG("%.2f, %.2f,%.2f", frustum.WorldRight().x, frustum.WorldRight().y, frustum.WorldRight().z);
		}
	}

	// Mouse wheel -----------------------
	 /*
	float zDelta = (float) App->input->GetMouseWheel();

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
	*/

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
float * ModuleCamera3D::GetOpenGLViewMatrix()
{
	static float4x4 m;
	
	m = frustum.ViewMatrix();
	m.Transpose();

	return (float*) m.v;
}

float * ModuleCamera3D::GetOpenGLProjectionMatrix()
{
	static float4x4 m;
	
	m = frustum.ProjectionMatrix();
	m.Transpose();

	return (float*) m.v;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
	//ViewMatrixInverse = inverse(ViewMatrix);
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
	following = nullptr;
}