#ifndef __RESOURCE_MODEL_H__
#define __RESOURCE_MODEL_H__

#include "Resource.h"
#include "Math.h"

struct aiScene;
struct aiNode;

class ResourceModel : public Resource
{
public:

    ResourceModel(UID id);
    virtual ~ResourceModel();

	void        Save                (Config& config) const override;
	void        Load                (const Config& config) override;

	bool        LoadInMemory        () override;
    void        ReleaseFromMemory   () override;
    bool        Save                (std::string& output) const;
    static UID  Import              (const aiScene* model, const std::vector<UID>& meshes, const std::vector<UID>& materials, const char* source_file);

private:

    void        GenerateNodes       (const aiScene* model, const aiNode* node, uint parent, const std::vector<UID>& meshes, const std::vector<UID>& materials);

private:

    struct Node
    {
        std::string         name;
        float4x4            transform = float4x4::identity;
        uint                parent    = 0;
        UID                 mesh      = 0;
        UID                 material  = 0;
    };

    std::vector<Node> nodes;

};

#endif /* __RESOURCE_MODEL_H__ */
