#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include <vector>
#include "Module.h"

struct aiScene;
struct aiNode;
struct aiMaterial;

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init();
	bool CleanUp();

	bool LoadScene(const char* file);
	void Draw() const;

private:

	void PrepareMaterial(const aiMaterial* material) const;
	void RecursiveDraw(const aiNode* node) const;

	const struct aiScene* scene = nullptr;
};

#endif // __MODULE_SCENE_H__