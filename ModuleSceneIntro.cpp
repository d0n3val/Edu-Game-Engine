#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	graphics = NULL;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	float road_width = 15.0f;
	float road_height = 5.0f;

	float circuit_x = 150.0f;
	float circuit_y = 80.0f;

	App->physics3D->AddBody(Plane(0, 1, 0, 0))->collision_listeners.add(this);

	c1.size.Set(road_width, road_height, circuit_x);
	c1.SetPos(0, road_height / 2, circuit_x / 2);
	App->physics3D->AddBody(c1, 0)->collision_listeners.add(this);

	c2.size.Set(circuit_y + road_width, road_height, road_width);
	c2.SetPos(-circuit_y / 2, road_height / 2, circuit_x + road_width / 2);
	App->physics3D->AddBody(c2, 0);

	c3.size.Set(road_width, road_height, circuit_x);
	c3.SetPos(-circuit_y, road_height / 2, circuit_x / 2);
	App->physics3D->AddBody(c3, 0);

	c4.size.Set(circuit_y + road_width, road_height, road_width);
	c4.SetPos(-circuit_y / 2, road_height / 2, -road_width / 2);
	App->physics3D->AddBody(c4, 0);

	y1.height = road_width / 2;
	y1.radius = 1.0f;
	y1.SetPos(-y1.height / 2, road_height, circuit_x / 6);
	App->physics3D->AddBody(y1, 0);

	y2.height = road_width / 2;
	y2.radius = 1.0f;
	y2.SetPos(y1.height / 2, road_height, (circuit_x / 6) * 2);
	App->physics3D->AddBody(y2, 0);

	y3.height = road_width / 2;
	y3.radius = 1.0f;
	y3.SetPos(-y1.height / 2, road_height, (circuit_x / 6) * 3);
	App->physics3D->AddBody(y3, 0);

	y4.height = road_width / 2;
	y4.radius = 1.0f;
	y4.SetPos(y1.height / 2, road_height, (circuit_x / 6) * 4);
	App->physics3D->AddBody(y4, 0);

	s1.radius = 7.0f;
	s1.SetPos(-circuit_y - y1.height / 2, road_height - s1.radius*0.9, circuit_x / 6);
	App->physics3D->AddBody(s1, 0);

	s2.radius = 7.0f;
	s2.SetPos(-circuit_y + y1.height / 2, road_height - s1.radius*0.9, (circuit_x / 6) * 2);
	App->physics3D->AddBody(s2, 0);

	s3.radius = 7.0f;
	s3.SetPos(-circuit_y - y1.height / 2, road_height - s1.radius*0.9, (circuit_x / 6) * 3);
	App->physics3D->AddBody(s3, 0);

	s4.radius = 7.0f;
	s4.SetPos(-circuit_y + y1.height / 2, road_height - s1.radius*0.9, (circuit_x / 6) * 4);
	App->physics3D->AddBody(s4, 0);

	App->camera->Move(vec3(0, 30, -30));

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update(float dt)
{
	Plane(0, 1, 0, 0).Render();

	c1.Render();
	c2.Render();
	c3.Render();
	c4.Render();

	y1.Render();
	y2.Render();
	y3.Render();
	y4.Render();

	s1.Render();
	s2.Render();
	s3.Render();
	s4.Render();

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	int a = 0;
}
