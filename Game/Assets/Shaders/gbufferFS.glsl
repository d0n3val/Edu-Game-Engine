#version 460

#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/common.glsl"
#include "/shaders/materialDefs.glsl"
#include "/shaders/vertexDefs.glsl"

in VertexOut fragment;
in flat int draw_id;

layout(location = 0)out vec4 albedo;
layout(location = 1)out vec4 specular;
layout(location = 2)out vec4 emissive;
layout(location = 3)out vec4 position;
layout(location = 4)out vec4 normal;

void packGBuffer(in PBR pbr)
{
    albedo.rgb        = pbr.diffuse;
    albedo.a          = pbr.shadow;
    specular.rgb      = pbr.specular;
    specular.a        = pbr.smoothness;
    emissive.rgb      = pbr.emissive;
    emissive.a        = pbr.occlusion;
    normal.rgb        = pbr.normal*0.5+0.5;
    position.rgb      = pbr.position;
}

void main()
{
    PBR pbr;

    getMaterial(pbr, draw_id, fragment.uv0, fragment.geom, fragment.shadowCoord);

    packGBuffer(pbr);
}
