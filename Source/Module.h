#ifndef __MODULE_H__
#define __MODULE_H__

#include <string.h>

class Application;
class Config;
struct PhysBody3D;
struct Event;

#define MODULE_NAME_LENGTH 25
class Module
{
private :
	bool enabled;
	char name[MODULE_NAME_LENGTH];

public:
	Module(const char* name, bool start_enabled = true) : enabled(start_enabled)
	{
		strcpy_s(this->name, MODULE_NAME_LENGTH, name);
	}

	virtual ~Module()
	{}

	const char* GetName() const
	{
		return name;
	}

	bool IsActive() const
	{
		return enabled;
	}

	void SetActive(bool active)
	{
		if (enabled != active)
		{
			enabled = active;
			if (active == true)
				Start();
			else
				CleanUp();
		}
	}

	virtual bool Init(Config* config = nullptr) 
	{
		return true; 
	}

	virtual bool Start(Config* config = nullptr)
	{
		return true;
	}

	virtual update_status PreUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status PostUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		return true; 
	}

	virtual void Save(Config* config) const
	{}

	virtual void Load(Config* config) 
	{}

	virtual void DebugDraw() 
	{}

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{ }

	virtual void ReceiveEvent(const Event& event)
	{ }
};

#endif // __MODULE_H__