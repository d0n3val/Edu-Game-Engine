#ifndef _MATERIAL_DEFS_GLSL_
#define _MATERIAL_DEFS_GLSL_

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/pbrDefs.glsl"
#include "/shaders/vertexDefs.glsl"

#define DIFFUSE_MAP_INDEX 0
#define BASECOLOR_MAP_INDEX 0

#define SPECULAR_MAP_INDEX 1
#define METALLICROUGH_MAP_INDEX 1

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
#define BASECOLOR_MAP_FLAG      0x00000001u

#define SPECULAR_MAP_FLAG       0x00000002u
#define METALLICROUGH_MAP_FLAG  0x00000002u

#define NORMAL_MAP_FLAG         0x00000004u
#define OCCLUSION_MAP_FLAG      0x00000008u
#define EMISSIVE_MAP_FLAG       0x00000010u
#define LIGHT_MAP_FLAG          0x00000020u
#define DETAIL_MASK_FLAG        0x00000040u
#define SECOND_DIFFUSE_FLAG     0x00000080u
#define SECOND_SPECULAR_FLAG    0x00000100u
#define SECOND_NORMAL_FLAG      0x00000200u
#define PLANAR_REFLECTION_FLAG  0x00000400u

#define SPECULAR_GLOSSINES_TYPE 0 
#define METALLIC_ROUGHNESS_TYPE 0 

struct TexHandle
{
    int index;
    float slice;
};

struct Material
{
    vec4      diffuseColor; // baseColor
    vec4      specularColor; // metallicRoughness
    vec4      emissiveColor; // emissive
    vec2      uv_tiling; // tiling
    vec2      uv_offset; // offset
    vec2      uv_secondary_tiling; // second tiling
    vec2      uv_secondary_offset; // second offset
    uint      type; 
    uint      batchIndex; 
    float     alphaTest;      
    uint      mask;           // mask
    sampler2D handles[MAP_COUNT];
};

readonly layout(std430, binding = MATERIAL_SSBO_BINDING) buffer Mats
{
    Material materials[];
};

layout(binding = MATERIAL_TEX_BINDING) uniform sampler2DArray textures[gl_MaxTextureImageUnits-8];

vec4 sampleTexture(in uint textureIndex, in vec2 uv, in int matIndex)
{
    return texture(materials[matIndex].handles[textureIndex], uv);
}

vec4 sampleTexture(in uint textureIndex, in vec2 uv, in Material material)
{
    return texture(material.handles[textureIndex], uv);
}

mat3 createTBN(const vec3 normal, const vec3 tangent, float sign)
{
    vec3 bitangent = sign*cross(normal, tangent);

    return mat3(tangent, bitangent, normal);
}

void getSpecularGlossinessMaterial(out PBR pbr, in Material material, in vec2 uv0, in GeomData geom)
{
    pbr.diffuse    = material.diffuseColor.rgb;
    pbr.specular   = material.specularColor.rgb;
    pbr.smoothness = material.specularColor.a;
    pbr.occlusion  = 1.0;
    pbr.emissive   = material.emissiveColor.rgb;
    pbr.alpha      = material.diffuseColor.a;

    uv0      = uv0*material.uv_tiling+material.uv_offset;
    vec2 uv1 = uv0*material.uv_secondary_tiling+material.uv_secondary_offset;

    if((material.mask & DIFFUSE_MAP_FLAG) != 0)
    {
        vec4 diffuse = sampleTexture(DIFFUSE_MAP_INDEX, uv0, material);
        pbr.diffuse *= diffuse.rgb;
        pbr.alpha    = diffuse.a;
    }

    if((material.mask & SPECULAR_MAP_FLAG) != 0)
    {
        vec4 tmp = sampleTexture(SPECULAR_MAP_INDEX, uv0, material);
        pbr.specular   = min(vec3(1.0), pbr.specular*tmp.rgb);
        pbr.smoothness = min(max(pbr.smoothness*tmp.a, 0.0), 1.0);
    }

    if((material.mask & OCCLUSION_MAP_FLAG) != 0)
    {
        pbr.occlusion *= sampleTexture(OCCLUSION_MAP_INDEX, uv0, material).r;
    }

    if((material.mask & EMISSIVE_MAP_FLAG) != 0)
    {
        pbr.emissive *= sampleTexture(EMISSIVE_MAP_INDEX, uv0, material).rgb;
    }

    vec3 tex_normal = vec3(0.0);
    bool has_tex_normal = false;

    if((material.mask & NORMAL_MAP_FLAG) != 0)
    {
        tex_normal = sampleTexture(NORMAL_MAP_INDEX, uv0, material).xyz*2.0-1.0;
        //tex_normal.z = sqrt(clamp(1-dot(tex_normal.xy, tex_normal.xy), 0.0, 1.0)); // unpack from two channel texture
        tex_normal.xy *= material.emissiveColor.a; // normal strength
        tex_normal = normalize(tex_normal);

        has_tex_normal = true;
    }
    
    if((material.mask & DETAIL_MASK_FLAG) != 0)
    {
        float blend = sampleTexture(DETAIL_MASK_MAP_INDEX, uv0, material).a;

        if((material.mask & SECOND_DIFFUSE_FLAG) != 0)
        {
            pbr.diffuse = mix(pbr.diffuse, sampleTexture(SECOND_DIFFUSE_MAP_INDEX, uv1, material).rgb, blend);
        }

        if((material.mask & SECOND_SPECULAR_FLAG) != 0)
        {
            vec4 spec  = sampleTexture(SECOND_SPECULAR_MAP_INDEX, uv1, material);
            pbr.specular   = mix(pbr.specular, spec.rgb, blend);
            pbr.smoothness = mix(pbr.smoothness, spec.a, blend);
        }

        if((material.mask & SECOND_NORMAL_FLAG) != 0)
        {
            vec3 second_tex_normal = sampleTexture(SECOND_NORMAL_MAP_INDEX, uv1, material).xyz*2.0-1.0;
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
        mat3 tbn = mat3(normalize(geom.tangent), normalize(geom.bitangent), normalize(geom.normal));
        pbr.normal = normalize(tbn*tex_normal);
    }
    else
    {
        pbr.normal = normalize(geom.normal);
    }

    pbr.position = geom.position;
    pbr.planarReflections = (material.mask & PLANAR_REFLECTION_FLAG) != 0 ? 1 : 0;

}

void getMetallicRoughnessMaterial(out PBR pbr, in Material material, in vec2 uv0, in GeomData geom)
{
    vec3 baseColor = material.diffuseColor.rgb;
    float metalness = material.specularColor.r;
    float roughness = material.specularColor.g;
    float normal_strength = material.specularColor.a;

    pbr.alpha = material.diffuseColor.a;
    pbr.occlusion = material.specularColor.b;
    pbr.emissive = material.emissiveColor.rgb;

    uv0 = uv0*material.uv_tiling+material.uv_offset;

    if((material.mask & BASECOLOR_MAP_FLAG) != 0)
    {
        vec4 baseColorSample = sampleTexture(BASECOLOR_MAP_INDEX, uv0, material);
        baseColor *= baseColorSample.rgb;
        pbr.alpha *= baseColorSample.a;
    }

    if((material.mask & METALLICROUGH_MAP_FLAG) != 0)
    {
        vec4 tmp = sampleTexture(METALLICROUGH_MAP_INDEX, uv0, material);

        metalness *= tmp.b;
        roughness *= tmp.g;
    }

    if((material.mask & OCCLUSION_MAP_FLAG) != 0)
    {
        pbr.occlusion *= sampleTexture(OCCLUSION_MAP_INDEX, uv0, material).r;
    }

    if((material.mask & EMISSIVE_MAP_FLAG) != 0)
    {
        pbr.emissive *= sampleTexture(EMISSIVE_MAP_INDEX, uv0, material).rgb;
    }


    pbr.diffuse    = baseColor*(1-metalness);
    pbr.specular   = mix(vec3(0.04), baseColor, metalness);
    pbr.smoothness = (1-roughness);
    pbr.smoothness *= pbr.smoothness;

    if((material.mask & NORMAL_MAP_FLAG) != 0)
    {
        vec3 tex_normal = sampleTexture(NORMAL_MAP_INDEX, uv0, material).xyz*2.0-1.0;
        tex_normal.xy *= normal_strength; 
        tex_normal = normalize(tex_normal);

        mat3 tbn = mat3(normalize(geom.tangent), normalize(geom.bitangent), normalize(geom.normal));

        pbr.normal = normalize(tbn*tex_normal);
    }
    else
    {
        pbr.normal = normalize(geom.normal);
    }

    pbr.position = geom.position;
    pbr.planarReflections = (material.mask & PLANAR_REFLECTION_FLAG) != 0 ? 1 : 0;
}

void getMaterial(out PBR pbr, in int matIndex, in vec2 uv0, in GeomData geom)
{
    Material material = materials[matIndex]; 

    if(material.type == SPECULAR_GLOSSINES_TYPE)
    {
        getSpecularGlossinessMaterial(pbr, material, uv0, geom);
    }
    else
    {
        getMetallicRoughnessMaterial(pbr, material, uv0, geom);
    }
}

#endif /* _MATERIAL_DEFS_GLSL_ */
