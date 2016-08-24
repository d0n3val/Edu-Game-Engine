#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Globals.h"
#include "Module.h"

class ResourceMesh;
struct aiMesh;

class ModuleMeshes : public Module
{
public:
	ModuleMeshes(bool start_enabled = true);
	~ModuleMeshes();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Load(ResourceMesh* resource);
	bool Import(const aiMesh* mesh, std::string& output) const;
	void GenerateVertexBuffer(const ResourceMesh* mesh);

	// Simple primitives
	bool LoadCube(ResourceMesh* resource);
	bool LoadSphere(ResourceMesh* resource);

private:
	bool Save(const ResourceMesh& mesh, std::string& output) const;
};

#endif // __MODULE_MESHES_H__