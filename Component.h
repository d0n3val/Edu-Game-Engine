#ifndef __COMPONENT_H__
#define __COMPONENT_H__

// Base class for all possible components a GameObject could have

enum ComponentTypes
{
	Invalid,
	Geometry,
	Material,
};

class GameObject;

class Component
{
public:
	Component(GameObject* container);
	virtual ~Component();

	void Activate();
	void DeActivate();
	bool IsActive() const;
	ComponentTypes GetType() const;

	virtual void OnActivate() {};
	virtual void OnDeActivate() {};

	virtual void OnStart() {};
	virtual void OnUpdate() {};
	virtual void OnFinish() {};

protected:
	ComponentTypes type = ComponentTypes::Invalid;
	bool active = false;
	GameObject* game_object = nullptr;
};

#endif // __COMPONENT_H__