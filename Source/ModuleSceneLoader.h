#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include <string>
#include "Module.h"
#include "Math.h"

struct aiScene;
struct aiNode;
struct aiMaterial;
struct aiMetadata;
class GameObject;

class ModuleSceneLoader : public Module
{
public:

	ModuleSceneLoader(bool start_enabled = true);
	~ModuleSceneLoader();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Import(const char* file, std::string& output);

private:

	void LoadMetaData(aiMetadata* const meta);
	void RecursiveCreateGameObjects(const aiScene* scene, const aiNode* node, GameObject* parent, const std::string& basePath, const std::string& file);
};

#endif // __MODULE_SCENE_H__