#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#define MAX_LIGHTS 8

class DDRenderInterfaceLegacyGL;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp();

	void OnResize(int width, int height);

	bool GetVSync() const;
	void SetVSync(bool vsync);

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	DDRenderInterfaceLegacyGL* debug_draw_interface = nullptr;
	bool vsync = false;
};

#endif // __MODULE_RENDERER_3D_H__