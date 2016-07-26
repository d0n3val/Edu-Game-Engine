#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include <string>
#include "Module.h"

struct aiScene;
struct aiNode;
struct aiMaterial;
class GameObject;

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init(Config* config = nullptr);
	bool CleanUp();

	bool LoadScene(const char* file);
	void Draw() const;

private:

	void RecursiveDrawGameObjects(const GameObject* go) const;
	void RecursiveCreateGameObjects(const aiNode* node, GameObject* parent, const std::string& basePath);

	const struct aiScene* scene = nullptr;
	GameObject* root = nullptr;
};

#endif // __MODULE_SCENE_H__