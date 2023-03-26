#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/cameraDefs.glsl"

uniform mat4 model;
uniform float time;
uniform vec2 tiling;
uniform vec2 offset;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_texcoord;
layout(location = 2) in vec3 vertex_color;

out vec2 uv;
out vec3 incolor;

void main()
{
    uv = vertex_texcoord*tiling+offset;
    uv.x -= time;
    incolor = vertex_color;
    gl_Position = proj*view*model*vec4(vertex_position, 1.0);
}
