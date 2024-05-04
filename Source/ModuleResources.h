#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Globals.h"
#include "Module.h"
#include "Resource.h"
#include <map>
#include <vector>

#define RESERVED_RESOURCES 7 // cube/sphere/cylinde/cone/pyramid primitives + checker texture

class Resource;
class LoaderAnimation;
class ResourceMesh;
class ResourceTexture;

class ModuleResources : public Module
{
public:

	ModuleResources(bool start_enabled = true);

	// Destructor
	~ModuleResources();

	bool Init(Config* config) override;
	bool Start(Config* config) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;

    void SaveTypedResources(Resource::Type type);
	void SaveResources() const;
	void LoadResources();
    void SaveResourcesTo(const char* path);

	Resource::Type TypeFromExtension(const char* extension) const;
	UID Find(const char* file_in_assets) const;
	UID ImportFileOutsideVFM(const char* full_path);
	UID ImportFile(const char* new_file_in_assets, Resource::Type type, bool force = false);
	UID ImportBuffer(const void* buffer, uint size, Resource::Type type, const char* source_file = nullptr);
	UID ImportTexture(const char* file_name, bool mipmaps, bool srgb, bool toCubemap);
	UID ImportAnimation(const char* file_name, uint first, uint last, const char* user_name, float scale);
    UID ImportModel(const char* file_name, float scale, const char* user_name);

	UID GenerateNewUID();
	const Resource* Get(UID uid) const;
	Resource *      Get(UID uid);

	ResourceTexture* GetTexture(UID uid) { Resource* res = Get(uid); return res && res->GetType() == Resource::texture ? reinterpret_cast<ResourceTexture*>(res) : nullptr; }
	const ResourceTexture* GetTexture(UID uid) const { const Resource* res = Get(uid); return res && res->GetType() == Resource::texture ? reinterpret_cast<const ResourceTexture*>(res) : nullptr; }

	Resource*   CreateNewResource (Resource::Type type, UID force_uid = 0);
	void        GatherResourceType(std::vector<const Resource*>& resources, Resource::Type type) const;

	void ReleaseFromMemory(UID uid);
    void RemoveResource(UID uid);
	
    const ResourceTexture* GetChecker() const { return checkers; }
    const ResourceTexture* GetWhiteFallback() const { return white_fallback; }
    const ResourceTexture* GetBlackFallback() const { return black_fallback; }
    const ResourceTexture* GetDefaultSkybox() const { return skybox; }
    const ResourceTexture* GetDefaultBlueNoise() const { return blueNoise;  }

	const ResourceMesh*    GetDefaultBox() const { return cube; }
	const ResourceMesh*    GetDefaultSphere() const { return sphere; }
	const ResourceMesh*    GetDefaultPlane() const { return plane; }
	const ResourceMesh*    GetDefaultCylinder() const { return cylinder; }
    const ResourceMesh*    GetDefaultCone() const {return cone; }

    const char* GetDirByType(Resource::Type type) const;

private:

	void LoadUID();
	void SaveUID() const;
    UID ImportSuccess(Resource::Type type, const char* file_name, const char* user_name, const std::string& output);
    bool LoadDefaultBlueNoise();
    bool LoadDefaultSkybox();
	bool LoadDefaultBox();
	bool LoadDefaultSphere();
    bool LoadDefaultRedImage();
	bool LoadDefaultPlane();
    bool LoadDefaultCylinder();
	bool LoadDefaultCone();

private:
	std::string asset_folder;
	UID last_uid = RESERVED_RESOURCES + 1; // reserve 1 for standard cube mesh
	std::map<UID, Resource*> resources;
	std::vector<Resource*> removed;

	// Presets
	ResourceMesh* cube = nullptr;
	ResourceMesh* sphere = nullptr;
	ResourceMesh* cone = nullptr;
	ResourceMesh* pyramid = nullptr;
	ResourceMesh* cylinder = nullptr;
	ResourceMesh* plane = nullptr;
	ResourceTexture* checkers = nullptr;
	ResourceTexture* skybox = nullptr;
    ResourceTexture* redImage = nullptr;
    ResourceTexture* white_fallback = nullptr;
    ResourceTexture* black_fallback = nullptr;
    ResourceTexture* blueNoise = nullptr;
};

#endif // __MODULERESOURCES_H__
