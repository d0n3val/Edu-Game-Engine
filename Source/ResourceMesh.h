#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"
#include "Math.h"
#include "HashString.h"

#include <string>

class Resource;
struct aiMesh;

class ResourceMesh : public Resource
{
	friend class ModuleMeshes;

public:
	ALIGN_CLASS_TO_16

	ResourceMesh(UID id);
	virtual ~ResourceMesh();


	void        Save        (Config& config) const override;
	void        Load        (const Config& config) override;

	bool        LoadInMemory() override;
    bool        Save        (std::string& output) const;
    static UID  Import      (const aiMesh* mesh, UID material, const char* source_file);

private:

    void GenerateAttribInfo (const aiMesh* mesh);
    void GenerateCPUBuffers (const aiMesh* mesh);
    void GenerateVBO        (bool dynamic);
    void GenerateBoneData   (const aiMesh* mesh);
    void GenerateVAO        ();

public:

	enum VertexAttribs
	{
		ATTRIB_TEX_COORDS_0 = 1 << 0,
		ATTRIB_NORMALS      = 1 << 1,
        ATTRIB_TANGENTS     = 1 << 2,
		ATTRIB_BONES        = 1 << 3,
	};

	struct Weight
	{
		unsigned vertex = 0;
		float weight    = 0.0f;
	};

	struct Bone
	{
		HashString	name;
		Weight*		weights     =  nullptr;
		unsigned	num_weights = 0;
		float4x4	bind		= float4x4::identity;
	};

    HashString  name;

    UID         mat_id              = 0;

    uint        vertex_size         = 0;
    uint        attribs             = 0;
    uint        texcoord_offset     = 0;
    uint        normal_offset       = 0;
    uint        tangent_offset      = 0;
	uint        bone_idx_offset		= 0;
    uint        bone_weight_offset  = 0;

    uint        num_vertices        = 0;
    float3*     src_vertices        = nullptr;
    float2*     src_texcoord0       = nullptr;
    float3*     src_normals         = nullptr;
    float3*     src_tangents        = nullptr;

    uint        num_indices 	    = 0;
    uint*       src_indices 	    = nullptr;

	uint        num_bones			= 0;
	Bone*       bones 		        = nullptr;
	float4x4*   palette				= 0;

    uint        vao 	            = 0;
    uint        vbo 	            = 0;
    uint        ibo 	            = 0;
};

#endif // __RESOURCE_MESH_H__
