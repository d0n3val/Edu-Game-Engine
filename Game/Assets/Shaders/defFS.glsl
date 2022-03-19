#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/materialDefs.glsl"
#include "/shaders/lighting.glsl"

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

in flat int draw_id;

out vec4 color;

void main()
{
    PBR pbr;
    getMaterial(pbr, draw_id, fragment.uv0, fragment.position, fragment.normal, fragment.tangent);

    color = Shading(pbr);
}
