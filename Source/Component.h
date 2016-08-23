#ifndef __COMPONENT_H__
#define __COMPONENT_H__

// Base class for all possible components a GameObject could have

#include "Config.h"
#include "Math.h"

class GameObject;
class Config;

class Component
{
public:

	enum Types
	{
		Geometry,
		Material,
		AudioListener,
		AudioSource,
		Camera,
		Bone,
		RigidBody,
		Animation,
		Unknown
	};

public:
	Component(GameObject* container, Types type);
	virtual ~Component();

	void SetActive(bool active);
	bool IsActive() const;
	Types GetType() const;
	const char* GetTypeStr() const;
	const GameObject* GetGameObject() const;
	GameObject* GetGameObject();

	// Queries
	virtual void GetBoundingBox(AABB& box) const {}

	// Events
	virtual void OnSave(Config& config) const = 0;
	virtual void OnLoad(Config* config) = 0;

	virtual void OnActivate() {};
	virtual void OnDeActivate() {};

	virtual void OnStart() {};
	virtual void OnUpdate() {};
	virtual void OnFinish() {};

	virtual void OnPlay() {};
	virtual void OnStop() {};
	virtual void OnPause() {};
	virtual void OnUnPause() {};

	virtual void OnUpdateTransform() {};
	virtual void OnDebugDraw() const {};

public:
	bool flag_for_removal = false;

protected:
	Types type = Types::Unknown;
	bool active = false;
	GameObject* game_object = nullptr;
};

#endif // __COMPONENT_H__