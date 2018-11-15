#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include <string>
#include <map>
#include "Module.h"
#include "Math.h"

struct aiBone;
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
	bool Start(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Import(const char* full_path, std::string& output);
	
	UID FindBoneFromLastImport(const char* name) const;

private:

    void GenerateMaterials  (const aiScene* scene, const char* file, std::vector<UID>& materials);
	void GenerateMeshes     (const aiScene* scene, const char* file, const std::vector<UID>& materials, std::vector<UID>& meshes);
	void GenerateGameObjects(const aiNode* src, GameObject* dst, const std::vector<UID>& meshes);

private:


	// TODO clean up this mess :(
	std::map<aiBone*, UID> mesh_bone;
	std::map<std::string, aiBone*> bones;
	std::map<const aiNode*, GameObject*> relations;
	std::map<std::string, UID> imported_bones;
};

#endif // __MODULE_SCENE_H__
