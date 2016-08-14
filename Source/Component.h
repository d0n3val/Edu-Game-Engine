#ifndef __COMPONENT_H__
#define __COMPONENT_H__

// Base class for all possible components a GameObject could have

#include "Config.h"

enum ComponentTypes
{
	Invalid,
	Geometry,
	Material,
	AudioListener,
	AudioSource,
	Camera,
	Bone
};

class GameObject;
class Config;

class Component
{
public:
	Component(GameObject* container);
	virtual ~Component();

	void SetActive(bool active);
	bool IsActive() const;
	ComponentTypes GetType() const;
	const GameObject* GetGameObject() const;

	virtual void OnSave(Config& config) const = 0;
	virtual void OnLoad(Config* config) = 0;

	virtual void OnActivate() {};
	virtual void OnDeActivate() {};

	virtual void OnStart() {};
	virtual void OnUpdate() {};
	virtual void OnFinish() {};

	virtual void OnUpdateTransform() {};

	virtual void OnDebugDraw() const {};

public:
	bool flag_for_removal = false;

protected:
	ComponentTypes type = ComponentTypes::Invalid;
	bool active = false;
	GameObject* game_object = nullptr;
};

#endif // __COMPONENT_H__