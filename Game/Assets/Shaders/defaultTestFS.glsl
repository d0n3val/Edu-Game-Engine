--- PREFIX

#version 440

--- DATA

/*
layout(std140, row_major) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec3 view_pos;
} camera;

*/

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

--- MAIN

void main()
{
    color = vec4(1.0);
}

