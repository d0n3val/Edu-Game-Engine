--- PREFIX

#version 440

--- DATA

layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;

in struct VertexOut
{
    vec3 normal;
    vec3 position;
} fragment;

--- MAIN

void main()
{
    position = position;
    normal = normalize(normal);
}

