#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

readonly layout(std430, row_major, binding = SKINNING_PALETTE_BINDING) buffer Palette
{
    mat4 palette[];
};

readonly layout(std430, binding = SKINNING_INDICES_BINDING) buffer Indices
{
    ivec4 indices[];
};

readonly layout(std430, binding = SKINNING_WEIGHTS_BINDING) buffer Weights
{
    vec4 weights[];
};

readonly layout(std430, binding = SKINNING_POSITIONS_BINDING) buffer InPositions
{
    vec4 in_positions[];
};

readonly layout(std430, binding = SKINNING_INNORMALS_BINDING) buffer InNormals
{
    vec4 in_normals[];
};

readonly layout(std430, binding = SKINNING_INTANGENTS_BINDING) buffer InTangents
{
    vec4 in_tangents[];
};

writeonly layout(std430, binding = SKINNING_OUTPOSITIONS_BINDING) buffer OutPositions
{
    vec4 out_positions[];
};

writeonly layout(std430, binding = SKINNING_OUTNORMALS_BINDING) buffer OutNormals
{
    vec4 out_normals[];
};

writeonly layout(std430, binding = SKINNING_OUTTANGENTS_BINDING) buffer OutTangents
{
    vec4 out_tangents[];
};


layout(location = SKINNING_NUM_VERTICES_LOCATION) uniform int numVertices;
layout(location = SKINNING_NUM_BONES_LOCATION) uniform int numBones;
layout(location = SKINNING_NUM_TARGETS_LOCATION) uniform int numTargets;
layout(location = SKINNING_TARGET_STRIDE_LOCATION) uniform int targetStride;
layout(location = SKINNING_NORMAL_STRIDE_LOCATION) uniform int normalStride;
layout(location = SKINNING_TANGENT_STRIDE_LOCATION) uniform int tangentStride;

readonly layout(std430, binding = SKINNING_MORPH_WEIGHTS_BINDING) buffer MorphWeights
{
    float morphWeights[];
};

layout(binding=SKINNING_MORPH_TARGET_BINDING) uniform samplerBuffer morphData;


vec3 MorphPosition(vec3 position, uint index)
{
    vec3 res = position;
    for(int i=0; i< numTargets; ++i)
    {
        int texelIndex = int(targetStride*i+index);
        res += texelFetch(morphData, texelIndex).xyz*morphWeights[i];
    }

    return res;
}

vec3 MorphNormal(vec3 normal, uint index)
{
    vec3 res = normal;
    for(int i=0; i< numTargets; ++i)
    {
        int texelIndex = int(targetStride*i+normalStride+index);
        res = normalize(res+texelFetch(morphData, texelIndex).xyz*morphWeights[i]);
    }

    return res;
}

vec3 MorphTangent(vec3 tangent, uint index)
{
    vec3 res = tangent;
    for(int i=0; i< numTargets; ++i)
    {
        int texelIndex = int(targetStride*i+tangentStride+index);
        res = normalize(res+texelFetch(morphData, texelIndex).xyz*morphWeights[i]);
    }

    return res;
}

layout(local_size_x = SKINNING_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uint index = gl_GlobalInvocationID.x;

    if(index < numVertices)
    {
        vec4 position = in_positions[index];
        vec4 normal = in_normals[index];
        vec4 tangent = in_tangents[index];

        // Do morph targets if there are targets
        if(numTargets > 0)
        {
            position = vec4(MorphPosition(position.xyz, index), 1.0);
            normal   = vec4(MorphNormal(normal.xyz, index), 0.0);
            tangent  = vec4(MorphTangent(tangent.xyz, index), 0.0);
        }

        // Do skinning if there are bones
        if(numBones > 0)
        {
            mat4 skin_transform = palette[indices[index][0]]*weights[index][0]+
                                palette[indices[index][1]]*weights[index][1]+
                                palette[indices[index][2]]*weights[index][2]+
                                palette[indices[index][3]]*weights[index][3];

            position = skin_transform*position;
            normal = skin_transform*normal;
            tangent = skin_transform*tangent;
        }

        out_positions[index] = position;
        out_normals[index] = normal;
        out_tangents[index] = tangent;
    }
}