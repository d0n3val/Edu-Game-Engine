#ifndef __MODULEEDITOR_H__
#define __MODULEEDITOR_H__

#include "Module.h"

struct nk_context;
union SDL_Event;

class ModuleEditor : public Module
{
public:
	ModuleEditor(bool start_enabled = true);
	~ModuleEditor();

	bool Init(Config* config = nullptr) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void BeginInput();
	void HandleInput(SDL_Event* event);
	void EndInput();

	void Draw();
	bool IsHovered();

private:

    struct nk_context *context = nullptr;
};

#endif // __MODULEEDITOR_H__