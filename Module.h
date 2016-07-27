#ifndef __MODULE_H__
#define __MODULE_H__

class Application;
class Config;
struct PhysBody3D;

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

	bool IsEnabled() const
	{
		return enabled;
	}

	void Enable()
	{
		if(enabled == false)
		{
			enabled = true;
			Start();
		}
	}

	void Disable()
	{
		if(enabled == true)
		{
			enabled = false;
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

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{ }
};

#endif // __MODULE_H__