#ifndef _MATERIAL_DEFS_GLSL_
#define _MATERIAL_DEFS_GLSL_

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/pbrDefs.glsl"
#include "/shaders/shadows.glsl"

#define DIFFUSE_MAP_INDEX 0
#define SPECULAR_MAP_INDEX 1
#define NORMAL_MAP_INDEX 2
#define OCCLUSION_MAP_INDEX 3
#define EMISSIVE_MAP_INDEX 4
#define LIGHT_MAP_INDEX 5
#define DETAIL_MASK_MAP_INDEX 6
#define SECOND_DIFFUSE_MAP_INDEX 7
#define SECOND_SPECULAR_MAP_INDEX 8
#define SECOND_NORMAL_MAP_INDEX 9
#define MAP_COUNT 10

#define DIFFUSE_MAP_FLAG        0x00000001u
#define SPECULAR_MAP_FLAG       0x00000002u
#define NORMAL_MAP_FLAG         0x00000004u
#define OCCLUSION_MAP_FLAG      0x00000008u
#define EMISSIVE_MAP_FLAG       0x00000010u
#define LIGHT_MAP_FLAG          0x00000020u
#define DETAIL_MASK_FLAG        0x00000040u
#define SECOND_DIFFUSE_FLAG     0x00000080u
#define SECOND_SPECULAR_FLAG    0x00000100u
#define SECOND_NORMAL_FLAG      0x00000200u

struct TexHandle
{
    int index;
    float slice;
};

struct Material
{
    vec4      diffuseColor;
    vec4      specularColor;
    vec4      emissiveColor;
    vec2      uv_tiling;
    vec2      uv_offset;
    vec2      uv_secondary_tiling;
    vec2      uv_secondary_offset;
    float     smoothness;
    float     normalStrength;
    float     alphaTest;
    uint      mapMask;
    TexHandle handles[MAP_COUNT];
};

readonly layout(std430, binding = MATERIAL_SSBO_BINDING) buffer Mats
{
    Material materials[];
};

layout(binding = MATERIAL_TEX_BINDING) uniform sampler2DArray textures[gl_MaxTextureImageUnits-8];

vec4 sampleTexture(in uint textureIndex, in vec2 uv, in int matIndex)
{
    TexHandle handle  = materials[matIndex].handles[textureIndex];

    return texture(textures[handle.index], vec3(uv, handle.slice));
}

mat3 createTBN(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal);
    vec3 bitangent     = cross(normal, ortho_tangent);

    return mat3(tangent, bitangent, normal);
}

void getMaterial(out PBR pbr, in int matIndex, in vec2 uv0, in vec3 vertexPosition, in vec3 vertexNormal, in vec3 vertexTangent, in vec3 shadowCoord)
{
    Material material = materials[matIndex]; 
    
    pbr.diffuse    = material.diffuseColor.rgb;
    pbr.specular   = material.specularColor.rgb;
    pbr.smoothness = material.smoothness;
    pbr.occlusion  = 1.0;
    pbr.emissive   = material.emissiveColor.rgb;
    pbr.alpha      = material.diffuseColor.a;

    uv0      = uv0*material.uv_tiling+material.uv_offset;
    vec2 uv1 = uv0*material.uv_secondary_tiling+material.uv_secondary_offset;

    if((material.mapMask & DIFFUSE_MAP_FLAG) != 0)
    {
        vec4 diffuse = sampleTexture(DIFFUSE_MAP_INDEX, uv0, matIndex);
        pbr.diffuse *= diffuse.rgb;
        pbr.alpha    = diffuse.a;
    }

    if((material.mapMask & SPECULAR_MAP_FLAG) != 0)
    {
        vec4 tmp = sampleTexture(SPECULAR_MAP_INDEX, uv0, matIndex);
        pbr.specular   *= tmp.rgb;
        pbr.smoothness = tmp.a;
    }

    if((material.mapMask & OCCLUSION_MAP_FLAG) != 0)
    {
        pbr.occlusion *= sampleTexture(OCCLUSION_MAP_INDEX, uv0, matIndex).r;
    }

    if((material.mapMask & EMISSIVE_MAP_FLAG) != 0)
    {
        pbr.emissive *= sampleTexture(EMISSIVE_MAP_INDEX, uv0, matIndex).rgb;
    }

    vec3 tex_normal = vec3(0.0);
    bool has_tex_normal = false;

    if((material.mapMask & NORMAL_MAP_FLAG) != 0)
    {
        tex_normal = sampleTexture(NORMAL_MAP_INDEX, uv0, matIndex).xyz*2.0-1.0;
        tex_normal.xy *= material.normalStrength;
        tex_normal = normalize(tex_normal);

        has_tex_normal = true;
    }
    

    if((material.mapMask & DETAIL_MASK_FLAG) != 0)
    {
        float blend = sampleTexture(DETAIL_MASK_MAP_INDEX, uv0, matIndex).a;

        if((material.mapMask & SECOND_DIFFUSE_FLAG) != 0)
        {
            pbr.diffuse = mix(pbr.diffuse, sampleTexture(SECOND_DIFFUSE_MAP_INDEX, uv1, matIndex).rgb, blend);
        }

        if((material.mapMask & SECOND_SPECULAR_FLAG) != 0)
        {
            vec4 spec  = sampleTexture(SECOND_SPECULAR_MAP_INDEX, uv1, matIndex);
            pbr.specular   = mix(pbr.specular, spec.rgb, blend);
            pbr.smoothness = mix(pbr.smoothness, spec.a, blend);
        }

        if((material.mapMask & SECOND_NORMAL_FLAG) != 0)
        {
            vec3 second_tex_normal = sampleTexture(SECOND_NORMAL_MAP_INDEX, uv1, matIndex).xyz*2.0-1.0;
            // \todo: second_tex_normal.xy *= material.normalStrength;
            second_tex_normal = normalize(second_tex_normal);

            if(has_tex_normal)
            {
                tex_normal = normalize(mix(tex_normal, second_tex_normal, blend));
            }
            else
            {
                tex_normal = second_tex_normal;
            }

            has_tex_normal = true;
        }
    }

    if(has_tex_normal)
    {
        mat3 tbn = createTBN(normalize(vertexNormal), normalize(vertexTangent));
        pbr.normal = normalize(tbn*tex_normal);
    }
    else
    {
        pbr.normal = normalize(vertexNormal);
    }

    pbr.position = vertexPosition;
    pbr.shadow = ComputeShadow(shadowCoord);
}

#endif /* _MATERIAL_DEFS_GLSL_ */