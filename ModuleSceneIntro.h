#pragma once
#include "Module.h"
#include "p2DynArray.h"
#include "Globals.h"

#define BOUNCER_TIME 200

struct PhysBody3D;
class Cube;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody3D* body1, PhysBody3D* body2);

public:

	SDL_Texture* graphics;
	PhysBody3D* ground;
	Cube c1, c2, c3, c4;
	Cylinder y1, y2, y3, y4;
	Sphere s1, s2, s3, s4;
};
