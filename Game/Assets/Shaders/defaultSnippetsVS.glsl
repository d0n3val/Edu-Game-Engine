--- PREFIX

#version 460

--- DATA

#ifdef DEPTH_PREPASS
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;
layout(location = 7) in int  draw_id_att;
#else
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;
layout(location = 5) in vec3 vertex_tangent;
layout(location = 6) in vec2 vertex_uv1;
layout(location = 7) in int  draw_id_att;
#endif 

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


#ifdef DEPTH_PREPASS
out struct VertexOut
{
    vec3 normal;
    vec3 position;
} fragment;
#else 
out struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;
#endif 

out flat int draw_id;

vec3 MorphPosition(vec3 position);
vec3 MorphNormal(vec3 position);
vec3 MorphTangent(vec3 position);

void TransformOutput();
void ShadowCoords();

--- MAIN

void main()
{
    TransformOutput();
    ShadowCoords();

    gl_Position = proj*view*vec4(fragment.position, 1.0);

#ifndef DEPTH_PREPASS
    fragment.uv0      = vertex_uv0;
    fragment.uv1      = vertex_uv1;
#endif
    draw_id  = draw_id_att;
}

--- MORPH

#define MAX_MORPH_TARGETS 128

uniform samplerBuffer morphBuffer;

layout(std140) uniform Morph
{
    int   target_stride;
    int   normals_stride;
    int   tangents_stride;
    float weights[MAX_MORPH_TARGETS];
    int   num_targets;
};

vec3 MorphPosition(vec3 position)
{
    vec3 res = position;
    for(int i=0; i< num_targets; ++i)
    {
        res += texelFetch(morphBuffer, target_stride*i+gl_VertexID).xyz*weights[i];
    }

    return res;
}

vec3 MorphNormal(vec3 normal)
{
    vec3 res = normal;
    for(int i=0; i< num_targets; ++i)
    {
        res = normalize(res+texelFetch(morphBuffer, target_stride*i+normals_stride+gl_VertexID).xyz*weights[i]);
    }

    return res;
}

vec3 MorphTangent(vec3 tangent)
{
    vec3 res = tangent;
    for(int i=0; i< num_targets; ++i)
    {
        res = normalize(res+texelFetch(morphBuffer, target_stride*i+tangents_stride+gl_VertexID).xyz*weights[i]);
    }

    return res;
}

--- NO_MORPH

vec3 MorphPosition(vec3 position)
{
    return position;
}

vec3 MorphNormal(vec3 normal)
{
    return normal;
}

vec3 MorphTangent(vec3 tangent)
{
    return tangent;
}

--- SKINING

#define MAX_BONES 64

layout(std140, row_major) uniform Skining
{
    mat4 palette[MAX_BONES];
};

void TransformOutput()
{
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

    mat4 model = models[draw_id_att];

    fragment.position = (model*skin_transform*vec4(MorphPosition(vertex_position), 1.0)).xyz;
    fragment.normal   = (model*skin_transform*vec4(MorphNormal(vertex_normal), 0.0)).xyz;
#ifndef DEPTH_PREPASS
    fragment.tangent  = (model*skin_transform*vec4(MorphTangent(vertex_tangent), 0.0)).xyz;
#endif 
}

--- NO_SKINING

void TransformOutput()
{
    mat4 model = models[draw_id_att];

    fragment.position = (model*vec4(MorphPosition(vertex_position), 1.0)).xyz;
    fragment.normal   = (model*vec4(MorphNormal(vertex_normal), 0.0)).xyz;
#ifndef DEPTH_PREPASS
    fragment.tangent  = (model*vec4(MorphTangent(vertex_tangent), 0.0)).xyz;
#endif
}

--- SHADOW 

#ifdef DEPTH_PREPASS

void ShadowCoords()
{
}

#else

#define CASCADE_COUNT 3

out vec4 shadow_coord[3];
uniform mat4 light_proj[CASCADE_COUNT];
uniform mat4 light_view[CASCADE_COUNT];

void ShadowCoords()
{
    for(uint i=0; i<3; ++i)
    {
        shadow_coord[i] = light_proj[i]*light_view[i]*vec4(fragment.position, 1.0);
        shadow_coord[i] /= shadow_coord[i].w;
        shadow_coord[i].xy = shadow_coord[i].xy*0.5+0.5;
    }
}

#endif 

--- NO_SHADOW

void ShadowCoords()
{
}

