#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/vertexDefs.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;
layout(location = NORMAL_ATTRIB_LOCATION) in vec3 vertex_normal;
layout(location = UV0_ATTRIB_LOCATION) in vec2 vertex_uv0;
layout(location = BONE_INDEX_ATTRIB_LOCATION) in ivec4 bone_indices;
layout(location = BONE_WEIGHT_ATTRIB_LOCATION) in vec4 bone_weights;
layout(location = TANGENT_ATTRIB_LOCATION) in vec3 vertex_tangent;
layout(location = UV1_ATTRIB_LOCATION) in vec2 vertex_uv1;
layout(location = DRAW_ID_ATTRIB_LOCATION) in int  draw_id_att;


readonly layout(std430, row_major, binding = MODEL_SSBO_BINDING) buffer Transforms
{
    mat4 models[];
};

struct PerInstance
{
    uint  numBones;
    uint  baseBone;
    uint  numTargets;
    uint  baseTarget;
    uint  baseTargetWeight;
    uint  targetStride;
    uint  normalsStride;
    uint  tangentsStride;
};

readonly layout(std430, binding = PERINSTANCE_SSBO_BINDING) buffer PerInstances
{
    PerInstance instanceInfo[];
};

layout(std430, row_major, binding = PALETTE_SSBO_BINDING) buffer Skinning 
{
    mat4 palette[];
};

layout(std430, binding = MORPH_WEIGHT_SSBO_BINDING) buffer MorphWeights
{
    float morphWeights[];
};

layout(binding=MORPH_TARGET_TBO_BINDING) uniform samplerBuffer morphData;

#ifdef CASCADE_SHADOWMAP
layout(location=SHADOW_VIEWPROJ_LOCATION) uniform mat4 shadowViewProj[NUM_CASCADES];
#else
layout(location=SHADOW_VIEWPROJ_LOCATION) uniform mat4 shadowViewProj;
#endif 

out VertexOut fragment;
out flat int draw_id;

vec3 MorphPosition(vec3 position);
vec3 MorphNormal(vec3 position);
vec3 MorphTangent(vec3 position);

void TransformOutput(out GeomData geom);
void computeShadowCoord(in vec3 position, out ShadowData shadow);

void main()
{
    GeomData geom;
    TransformOutput(geom);

    gl_Position = proj*view*vec4(geom.position, 1.0);

    fragment.geom = geom;
    fragment.uv0  = vertex_uv0;
    fragment.uv1  = vertex_uv1;
    draw_id       = draw_id_att;

#ifndef SHADOW_MAP
    ShadowData shadow;
    computeShadowCoord(geom.position, shadow);

    fragment.shadow = shadow;
#endif 
}

#ifndef SHADOW_MAP
void computeShadowCoord(in vec3 position, out ShadowData shadow)
{

#ifdef CASCADE_SHADOWMAP
    for(int i=0; i< NUM_CASCADES; ++i)
    {
        vec4 coord = shadowViewProj[i]*vec4(position, 1.0);
        coord.xyz /= coord.w;
        coord.xy = coord.xy*0.5+0.5;

        shadow.shadowCoord[i] = coord.xyz;
    }
#else 
    vec4 coord = shadowViewProj*vec4(position, 1.0);
    coord.xyz /= coord.w;
    coord.xy = coord.xy*0.5+0.5;

    shadow.shadowCoord = coord.xyz;
#endif 

}
#endif 

vec3 MorphPosition(vec3 position)
{
    PerInstance instance = instanceInfo[draw_id_att];

    vec3 res = position;
    for(int i=0; i< instance.numTargets; ++i)
    {
        int texelIndex = int(instance.baseTarget+instance.targetStride*i+gl_VertexID);
        res += texelFetch(morphData, texelIndex).xyz*morphWeights[i];
    }

    return res;
}

vec3 MorphNormal(vec3 normal)
{
    PerInstance instance = instanceInfo[draw_id_att];

    vec3 res = normal;
    for(int i=0; i< instance.numTargets; ++i)
    {
        int texelIndex = int(instance.baseTarget+instance.targetStride*i+instance.normalsStride+gl_VertexID);
        res = normalize(res+texelFetch(morphData, texelIndex).xyz*morphWeights[i]);
    }

    return res;
}

vec3 MorphTangent(vec3 tangent)
{
    PerInstance instance = instanceInfo[draw_id_att];

    vec3 res = tangent;
    for(int i=0; i< instance.numTargets; ++i)
    {
        int texelIndex = int(instance.baseTarget+instance.targetStride*i+instance.tangentsStride+gl_VertexID);
        res = normalize(res+texelFetch(morphData, texelIndex).xyz*morphWeights[i]);
    }

    return res;
}

void TransformOutput(out GeomData geom)
{
    PerInstance instance = instanceInfo[draw_id_att];
    mat4 model = models[draw_id_att];

    mat3 normalMat = transpose(inverse(mat3(model)));

    if(instance.numTargets > 0) // MorphTargets
    {
        geom.position = MorphPosition(vertex_position);
        geom.normal   = MorphNormal(vertex_normal);
        geom.tangent  = MorphTangent(vertex_tangent);
    }
    else // No MorphTargets
    {
        geom.position = vertex_position;
        geom.normal   = vertex_normal;
        geom.tangent  = vertex_tangent;
    }

    if(instance.numBones > 0) // Skinning 
    {
        mat4 skin_transform = palette[instance.baseBone+bone_indices[0]]*bone_weights[0]+palette[instance.baseBone+bone_indices[1]]*bone_weights[1]+
                              palette[instance.baseBone+bone_indices[2]]*bone_weights[2]+palette[instance.baseBone+bone_indices[3]]*bone_weights[3];

        geom.position = (model*skin_transform*vec4(geom.position, 1.0)).xyz;
        geom.normal   = normalMat*mat3(skin_transform)*geom.normal;
        geom.tangent  = normalMat*mat3(skin_transform)*geom.tangent;
    }
    else // No Skinning
    {
        geom.position = (model*vec4(geom.position, 1.0)).xyz;
        geom.normal   = normalMat*geom.normal;
        geom.tangent  = normalMat*geom.tangent;
    }
}
