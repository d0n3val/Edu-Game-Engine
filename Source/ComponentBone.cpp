#include "ComponentBone.h"
#include "Globals.h"
#include "GameObject.h"
#include "Math.h"
#include "DebugDraw.h"
#include "Color.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
ComponentBone::ComponentBone(GameObject* container) : Component(container)
{
	type = ComponentTypes::Bone;
}

// ---------------------------------------------------------
ComponentBone::~ComponentBone()
{
	RELEASE_ARRAY(weigth_indices);
	RELEASE_ARRAY(weigths);
}

// ---------------------------------------------------------
void ComponentBone::OnSave(Config & config) const
{
	config.AddInt("Num Weigths", num_weigths);
	//config.AddArrayFloat("Offset", &offset.v[0][0], 16);
	//config.AddArrayInt("Indices", (const int*) weigth_indices, num_weigths);
	//config.AddArrayFloat("Weigths", weigths, num_weigths);
}

// ---------------------------------------------------------
void ComponentBone::OnLoad(Config * config)
{
	num_weigths = config->GetInt("Num Weigths");
}

void ComponentBone::OnDebugDraw() const
{
	math::LineSegment segment;
	segment.a = game_object->GetGlobalPosition();

	for (list<GameObject*>::const_iterator it = game_object->childs.begin(); it != game_object->childs.end(); ++it)
	{
		segment.b = (*it)->GetGlobalPosition();
		DebugDraw(segment, Blue);
	}
}
