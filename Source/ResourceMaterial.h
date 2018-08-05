#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"

class ResourceMaterial : public Resource
{
public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool        LoadInMemory   () override;
    bool        Save           (string& output) const;

    static UID  Import          (const aiMaterial* material, const char* source_file);

public:

    float4      ambient       = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4      diffuse       = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4      specular      = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float       shininess     = 64.0f;
    UID         albedo_map    = 0;
    UID         normal_map    = 0;
    UID         specular_map  = 0;
    UID         occlusion_map = 0;
    bool        cast_shadows  = true;
    bool        recv_shadows  = true;
};


