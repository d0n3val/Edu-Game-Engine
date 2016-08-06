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

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	bool CleanUp() override;

	bool LoadScene(const char* file);
	void LoadMetaData(aiMetadata* const meta);
	void Draw() const;

	const GameObject* GetRoot() const;
	GameObject* GetRoot();

	GameObject* CreateGameObject(GameObject * parent, const float3& pos, const float3& scale, const Quat& rot, const char* name = nullptr);

private:

	void RecursiveDrawGameObjects(const GameObject* go) const;
	void RecursiveCreateGameObjects(const aiNode* node, GameObject* parent, const std::string& basePath);

	const struct aiScene* scene = nullptr;
	GameObject* root = nullptr;
};

#endif // __MODULE_SCENE_H__