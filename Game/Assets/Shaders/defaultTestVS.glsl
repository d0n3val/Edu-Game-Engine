--- PREFIX

#version 440

--- DATA

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;
layout(location = 5) in vec3 vertex_tangent;
layout(location = 6) in vec2 vertex_uv1;

layout(std140, row_major) uniform Camera 
{
    mat4 proj;
    mat4 view;
    //vec3 view_pos ;
} camera;


uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

--- MAIN

void main()
{
    //gl_Position = camera.proj*camera.view*model*vec4(vertex_position, 1.0);
    gl_Position = camera.proj*camera.view*model*vec4(vertex_position, 1.0);
}

