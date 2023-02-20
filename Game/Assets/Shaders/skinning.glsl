#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"


readonly layout(std430, row_major, binding = SKINNING_PALETTE_BINDING) buffer Palette
{
    mat4 palette[];
};

readonly layout(std430, row_major, binding = SKINNING_INDICES_BINDING) buffer Indices
{
    ivec4 indices[];
};

readonly layout(std430, row_major, binding = SKINNING_WEIGHTS_BINDING) buffer Weights
{
    vec4 weights[];
};

readonly layout(std430, row_major, binding = SKINNING_POSITIONS_BINDING) buffer InPositions
{
    vec4 in_positions[];
};

readonly layout(std430, row_major, binding = SKINNING_INNORMALS_BINDING) buffer InNormals
{
    vec4 in_normals[];
};

readonly layout(std430, row_major, binding = SKINNING_INTANGENTS_BINDING) buffer InTangents
{
    vec4 in_tangents[];
};

writeonly layout(std430, row_major, binding = SKINNING_OUTPOSITIONS_BINDING) buffer OutPositions
{
    vec4 out_positions[];
};

writeonly layout(std430, row_major, binding = SKINNING_OUTNORMALS_BINDING) buffer OutNormals
{
    vec4 out_normals[];
};

writeonly layout(std430, row_major, binding = SKINNING_OUTTANGENTS_BINDING) buffer OutTangents
{
    vec4 out_tangents[];
};

layout(location = SKINNING_NUM_VERTICES_LOCATION) uniform int numVertices;

layout(local_size_x = SKINNING_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uint index = gl_GlobalInvocationID.x;

    if(index < numVertices)
    {
        mat4 skin_transform = palette[indices[index][0]]*weights[index][0]+
                            palette[indices[index][1]]*weights[index][1]+
                            palette[indices[index][2]]*weights[index][2]+
                            palette[indices[index][3]]*weights[index][3];

        out_positions[index] = skin_transform*in_positions[index];
        out_normals[index] = skin_transform*in_normals[index];
        out_tangents[index] = skin_transform*in_tangents[index];
    }
}