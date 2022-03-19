#version 460

#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/common.glsl"
#include "/shaders/materialDefs.glsl"

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

in flat int draw_id;

layout(location = 0)out vec4 albedo;
layout(location = 1)out vec4 specular;
layout(location = 2)out vec4 emissive;
layout(location = 3)out vec4 position;
layout(location = 4)out vec4 normal;

void packGBuffer(in PBR pbr)
{
    albedo.rgb   = pbr.diffuse;
    specular.rgb = pbr.specular;
    specular.a   = pbr.smoothness;
    emissive.rgb = pbr.emissive;
    emissive.a   = pbr.occlusion;
    normal.rgb   = pbr.normal;
    position.rgb = pbr.position;
}

void main()
{
    PBR pbr;

    getMaterial(pbr, draw_id, fragment.uv0, fragment.position, fragment.normal, fragment.tangent);
    packGBuffer(pbr);
}
