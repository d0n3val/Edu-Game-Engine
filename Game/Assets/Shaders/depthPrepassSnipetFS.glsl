--- PREFIX

#version 440

--- DATA

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

layout(std140, row_major) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec3 view_pos;
} camera;

in struct VertexOut
{
    vec3 normal;
    vec3 position;
} fragment;

--- MAIN

void main()
{
    position = camera.view*vec4(fragment.position, 1.0);
    normal = vec4(mat3(camera.view)*normalize(fragment.normal), 1.0);
}

