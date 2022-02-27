#version 460

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;
layout(location = 5) in vec3 vertex_tangent;
layout(location = 6) in vec2 vertex_uv1;
layout(location = 7) in int  draw_id_att;

layout(std140, row_major, binding = 0) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec4 view_pos;
};

readonly layout(std430, row_major, binding = 10) buffer Transforms
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

readonly layout(std430, binding = 15) buffer PerInstances
{
    PerInstance instanceInfo[];
};

layout(std430, row_major, binding = 16) buffer Skining
{
    mat4 palette[];
};

layout(std430, binding = 17) buffer MorphWeights
{
    float morphWeights[];
};

layout(binding=13) uniform samplerBuffer morphData;

out struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

out flat int draw_id;

vec3 MorphPosition(vec3 position);
vec3 MorphNormal(vec3 position);
vec3 MorphTangent(vec3 position);

void TransformOutput();

void main()
{
    TransformOutput();

    gl_Position = proj*view*vec4(fragment.position, 1.0);

    fragment.uv0 = vertex_uv0;
    fragment.uv1 = vertex_uv1;
    draw_id      = draw_id_att;
}

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

void TransformOutput()
{
    PerInstance instance = instanceInfo[draw_id_att];
    mat4 model = models[draw_id_att];

    if(instance.numTargets > 0) // MorphTargets
    {
        fragment.position = MorphPosition(vertex_position);
        fragment.normal   = MorphNormal(vertex_normal);
        fragment.tangent  = MorphTangent(vertex_tangent);
    }
    else // No MorphTargets
    {
        fragment.position = vertex_position;
        fragment.normal   = vertex_normal;
        fragment.tangent  = vertex_tangent;
    }

    if(instance.numBones > 0) // Skinning 
    {
        mat4 skin_transform = palette[instance.baseBone+bone_indices[0]]*bone_weights[0]+palette[instance.baseBone+bone_indices[1]]*bone_weights[1]+
                              palette[instance.baseBone+bone_indices[2]]*bone_weights[2]+palette[instance.baseBone+bone_indices[3]]*bone_weights[3];

        mat4 model = models[draw_id_att];

        fragment.position = (model*skin_transform*vec4(fragment.position, 1.0)).xyz;
        fragment.normal   = (model*skin_transform*vec4(fragment.normal, 0.0)).xyz;
        fragment.tangent  = (model*skin_transform*vec4(fragment.tangent, 0.0)).xyz;
    }
    else // No Skinning
    {
        fragment.position = (model*vec4(fragment.position, 1.0)).xyz;
        fragment.normal   = (model*vec4(fragment.normal, 0.0)).xyz;
        fragment.tangent  = (model*vec4(fragment.tangent, 0.0)).xyz;
    }
}
