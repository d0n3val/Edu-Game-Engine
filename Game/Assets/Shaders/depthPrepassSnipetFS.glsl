--- PREFIX

#version 440

--- DATA

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

in struct VertexOut
{
    vec3 normal;
    vec3 position;
} fragment;

--- MAIN

// \todo: normal mapping

void main()
{
    position = vec4(fragment.position, 1.0);
    normal = vec4(normalize(fragment.normal)*0.5+0.5, 1.0);
}

