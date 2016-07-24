#ifndef __COMPONENT_H__
#define __COMPONENT_H__

// Base class for all possible components a GameObject could have

class GameObject;

class Component
{
public:
	Component(GameObject* container);
	~Component();

	void Activate();
	void DeActivate();
	bool IsActive() const;

	virtual void OnActivate() {};
	virtual void OnDeActivate() {};

	virtual void OnStart() {};
	virtual void OnUpdate() {};
	virtual void OnFinish() {};

private:
	bool active = false;
	GameObject* game_object = nullptr;
};

#endif // __COMPONENT_H__