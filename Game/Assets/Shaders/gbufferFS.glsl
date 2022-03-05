#version 460

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

layout(std140, row_major) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec4 view_pos;
};

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

readonly layout(std430, binding = 5) buffer Mats
{
    Material materials[];
};

layout(binding = 0) uniform sampler2DArray textures[gl_MaxTextureImageUnits-8];

uniform sampler2D ambientOcclusion;
layout(binding = 10) uniform samplerCube diffuseIBL;
layout(binding = 11) uniform samplerCube prefilteredIBL;
layout(binding = 12) uniform sampler2D   environmentBRDF;
layout(location = 64) uniform int        prefilteredLevels;

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

in flat int draw_id;

layout(location = 0)out vec4 albedo;
layout(location = 1)out vec4 specular;
layout(location = 2)out vec4 emissive;
layout(location = 3)out vec4 position;
layout(location = 4)out vec4 normal;

struct MaterialPBR
{
    vec3  diffuse;
    vec3  specular;
    vec3  emissive;
    vec3  normal;
    float smoothness;
    float occlusion;
};

//////////////////// FUNCTIONS ////////////////////////

mat3 createTBN(const vec3 normal, const vec3 tangent);
void getMaterial(out MaterialPBR pbr);
void packGBuffer(in MaterialPBR pbr);
vec4 sampleTexture(uint textureIndex, vec2 uv);

void main()
{
    MaterialPBR pbr;

    getMaterial(pbr);
    packGBuffer(pbr);
}

void packGBuffer(in MaterialPBR pbr)
{
    albedo.rgb   = pbr.diffuse;
    specular.rgb = pbr.specular;
    specular.a   = pbr.smoothness;
    emissive.rgb = pbr.emissive;
    emissive.a   = pbr.occlusion;
    normal.rgb   = pbr.normal;
    position.rgb = fragment.position;
}

vec4 sampleTexture(uint textureIndex, vec2 uv)
{
    TexHandle handle  = materials[draw_id].handles[textureIndex];

    return texture(textures[handle.index], vec3(uv, handle.slice));
}

void getMaterial(out MaterialPBR pbr)
{
    Material material = materials[draw_id]; 
    
    pbr.diffuse    = material.diffuseColor.rgb;
    pbr.specular   = material.specularColor.rgb;
    pbr.smoothness = material.smoothness;
    pbr.occlusion  = 1.0;
    pbr.emissive   = material.emissiveColor.rgb;

    vec2 uv0   = fragment.uv0*material.uv_tiling+material.uv_offset;
    vec2 uv1   = fragment.uv0*material.uv_secondary_tiling+material.uv_secondary_offset;

    if((material.mapMask & DIFFUSE_MAP_FLAG) != 0)
    {
        pbr.diffuse *= sampleTexture(DIFFUSE_MAP_INDEX, uv0).rgb;
    }

    if((material.mapMask & SPECULAR_MAP_FLAG) != 0)
    {
        vec4 tmp = sampleTexture(SPECULAR_MAP_INDEX, uv0);
        pbr.specular   *= tmp.rgb;
        pbr.smoothness *= tmp.a;
    }

    if((material.mapMask & OCCLUSION_MAP_FLAG) != 0)
    {
        pbr.occlusion *= sampleTexture(OCCLUSION_MAP_INDEX, uv0).r;
    }

    if((material.mapMask & EMISSIVE_MAP_FLAG) != 0)
    {
        pbr.emissive *= sampleTexture(EMISSIVE_MAP_INDEX, uv0).rgb;
    }

    vec3 tex_normal = vec3(0.0);
    bool has_tex_normal = false;

    if((material.mapMask & NORMAL_MAP_FLAG) != 0)
    {
        tex_normal = sampleTexture(NORMAL_MAP_INDEX, uv0).xyz*2.0-1.0;
        tex_normal.xy *= material.normalStrength;
        tex_normal = normalize(tex_normal);

        has_tex_normal = true;
    }
    

    if((material.mapMask & DETAIL_MASK_FLAG) != 0)
    {
        float blend = sampleTexture(DETAIL_MASK_MAP_INDEX, fragment.uv0).a;

        if((material.mapMask & SECOND_DIFFUSE_FLAG) != 0)
        {
            pbr.diffuse = mix(pbr.diffuse, sampleTexture(SECOND_DIFFUSE_MAP_INDEX, uv1).rgb, blend);
        }

        if((material.mapMask & SECOND_SPECULAR_FLAG) != 0)
        {
            vec4 spec  = sampleTexture(SECOND_SPECULAR_MAP_INDEX, uv1);
            pbr.specular   = mix(pbr.specular, spec.rgb, blend);
            pbr.smoothness = mix(pbr.smoothness, spec.a, blend);
        }

        if((material.mapMask & SECOND_NORMAL_FLAG) != 0)
        {
            vec3 second_tex_normal = sampleTexture(SECOND_NORMAL_MAP_INDEX, uv1).xyz*2.0-1.0;
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
        mat3 tbn = createTBN(normalize(fragment.normal), normalize(fragment.tangent));
        pbr.normal = normalize(tbn*tex_normal);
    }
    else
    {
        pbr.normal = normalize(fragment.normal);
    }
}

mat3 createTBN(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal);
    vec3 bitangent     = cross(normal, ortho_tangent);

    return mat3(tangent, bitangent, normal);
}
