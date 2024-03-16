#version 460

#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/vertexDefs.glsl"
#include "/shaders/materialDefs.glsl"

in VertexOut fragment;
in flat int draw_id;

out vec4 color;

void main()
{
    Material material = materials[draw_id]; 
    color.r = float(draw_id);
    color.g = float(material.batchIndex);
}
