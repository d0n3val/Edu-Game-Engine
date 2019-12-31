#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"
#include "Math.h"
#include "HashString.h"
#include "utils/SimpleBinStream.h"

#include <string>

class Resource;
struct aiMesh;
struct par_shapes_mesh_s;

namespace Thekla { struct Atlas_Output_Mesh; }

class ResourceMesh : public Resource
{
	friend class ModuleMeshes;

public:
	ALIGN_CLASS_TO_16

	ResourceMesh(UID id);
	virtual ~ResourceMesh();


	void        Save                (Config& config) const override;
	void        Load                (const Config& config) override;

    void        UpdateUniforms      (const float4x4* skin_palette) const;
    void        Draw                () const;

	bool        LoadInMemory        () override;
    void        ReleaseFromMemory   () override;
    bool        Save                ();
    bool        Save                (std::string& output) const;
    static UID  Import              (const aiMesh* mesh, const char* source_file);

    static UID  LoadSphere          (const char* sphere_name, float size, unsigned slices, unsigned stacks);
    static UID  LoadTorus           (const char* torus_name, float inner_r, float outer_r, unsigned slices, unsigned stacks);
    static UID  LoadCube            (const char* cube_name, float size);
    static UID  LoadCylinder        (const char* cylinder_name, float height, float radius, unsigned slices, unsigned stacks);
    static UID  LoadPlane           (const char* plane_name, float width, float height, unsigned slices, unsigned stacks); 

    void        GenerateTexCoord1   ();

private:

    static UID  Generate            (const char* shape_name, par_shapes_mesh_s* shape);
    void        GenerateAttribInfo  ();
    void        GenerateCPUBuffers  (const Thekla::Atlas_Output_Mesh* atlas);
    void        GenerateCPUBuffers  (const aiMesh* mesh);
	void        GenerateCPUBuffers  (par_shapes_mesh_s* shape);
    void        GenerateVBO         ();
    void        GenerateBoneData    (const aiMesh* mesh);
    void        GenerateVAO         ();
    void        GenerateTangentSpace();
    void        Clear               ();
    bool        Save                (const char* source, std::string& output);
	void        SaveToStream        (simple::mem_ostream<std::true_type>& write_stream) const;

public:

	enum VertexAttribs
	{
		ATTRIB_TEX_COORDS_0 = 1 << 0,
		ATTRIB_NORMALS      = 1 << 1,
        ATTRIB_TANGENTS     = 1 << 2,
		ATTRIB_BONES        = 1 << 3,
		ATTRIB_TEX_COORDS_1 = 1 << 4,
	};

	struct Bone
	{
		HashString	name;
		float4x4	bind = float4x4::identity;
	};

    HashString  name;

    uint        vertex_size         = 0;
    uint        attribs             = 0;
    uint        texcoord0_offset    = 0;
    uint        texcoord1_offset    = 0;
    uint        normal_offset       = 0;
    uint        tangent_offset      = 0;
	uint        bone_idx_offset		= 0;
    uint        bone_weight_offset  = 0;

    uint        num_vertices        = 0;
    float3*     src_vertices        = nullptr;
    float2*     src_texcoord0       = nullptr;
    float2*     src_texcoord1       = nullptr;
    float3*     src_normals         = nullptr;
    float3*     src_tangents        = nullptr;
    unsigned*   src_bone_indices    = nullptr;
    float4*     src_bone_weights    = nullptr;

    uint        num_indices 	    = 0;
    uint*       src_indices 	    = nullptr;

	uint        num_bones			= 0;
	Bone*       bones 		        = nullptr;
    bool        static_mesh         = false;

    uint        vao 	            = 0;
    uint        vbo 	            = 0;
    uint        ibo 	            = 0;

	AABB        bbox;
};

#endif // __RESOURCE_MESH_H__
