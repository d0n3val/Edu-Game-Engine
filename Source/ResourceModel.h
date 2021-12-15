#ifndef __RESOURCE_MODEL_H__
#define __RESOURCE_MODEL_H__

#include "Resource.h"
#include "Math.h"
#include "utils/SimpleBinStream.h"

struct aiScene;
struct aiNode;

class ResourceModel : public Resource
{
public:

    struct MeshRenderer
    {
        UID  mesh       = 0;
        UID  material   = 0;
    };

    struct Node
    {
		Node() { ; }
		Node(const Node& o) = default;
        Node(Node&& o) = default;
		Node& operator=(const Node& o) = default;
		Node& operator=(Node&& o) = default;

        std::string               name;
        float4x4                  transform = float4x4::identity;
        uint                      parent    = 0;
        std::vector<MeshRenderer> renderers;
    };

public:

    ResourceModel(UID id);
    virtual ~ResourceModel();

	void        Save                (Config& config) const override;
	void        Load                (const Config& config) override;

	bool        LoadInMemory        () override;
    void        ReleaseFromMemory   () override;

    bool        Save                ();
    bool        Save                (std::string& output) const;
	static bool Import              (const char* full_path, float scale, std::string& output);

    unsigned    GetNumNodes         () const { return unsigned(nodes.size()); }
    const Node& GetNode             (uint index) const { return nodes[index]; }

private:

    void        GenerateNodes      (const aiScene* model, const aiNode* node, uint parent, const std::vector<UID>& meshes, const std::vector<UID>& materials, float scale);
    void        GenerateMaterials  (const aiScene* scene, const char* file, std::vector<UID>& materials);
	void        GenerateMeshes     (const aiScene* scene, const char* file, std::vector<UID>& meshes, float scale);
	void        SaveToStream       (simple::mem_ostream<std::true_type>& write_stream) const;

private:

    std::vector<Node> nodes;

};

#endif /* __RESOURCE_MODEL_H__ */
