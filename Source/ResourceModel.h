#ifndef __RESOURCE_MODEL_H__
#define __RESOURCE_MODEL_H__

#include "Resource.h"
#include "Math.h"

struct aiScene;

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
    static UID  Import              (const aiScene* model, UID material, const char* source_file);

public:

    struct Node
    {
        std::string         name;
        float4x4            transform;
        uint                parent;
        std::vector<uint>   childs;
        std::vector<UID>    meshes;
    };

    std::vector<Node> nodes;

};

#endif /* __RESOURCE_MODEL_H__ */
