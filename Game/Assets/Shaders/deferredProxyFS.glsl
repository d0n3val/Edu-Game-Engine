#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/lighting.glsl"

layout(binding = GBUFFER_ALBEDO_TEX_BINDING) uniform sampler2D albedo;
layout(binding = GBUFFER_SPECULAR_TEX_BINDING) uniform sampler2D specular;
layout(binding = GBUFFER_EMISSIVE_TEX_BINDING) uniform sampler2D emissive;
layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;
layout(binding = GBUFFER_NORMAL_TEX_BINDING) uniform sampler2D normal;
layout(binding = SSAO_TEX_BINDING) uniform sampler2D ssao;

in vec2 uv;
in flat int draw_id;
out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

void unpackGBuffer(out PBR pbr)
{
    vec4 albedoSmp   = texture(albedo, uv);
    vec4 specularSmp = texture(specular, uv);
    vec4 emissiveSmp = texture(emissive, uv);
    vec4 positionSmp = texture(position, uv);
    vec4 normalSmp   = texture(normal, uv);

    pbr.diffuse    = albedoSmp.rgb;
    pbr.shadow     = 1.0;
    pbr.specular   = specularSmp.rgb;
    pbr.emissive   = emissiveSmp.rgb;
    pbr.normal     = normalize(normalSmp.rgb*2.0-1.0);
    pbr.position   = positionSmp.rgb;
    pbr.smoothness = specularSmp.a;
    pbr.occlusion  = emissiveSmp.a;
}

void sampleSSAO(inout PBR pbr)
{
    pbr.occlusion *= texture(ssao, uv).r;
}

void main()
{
    PBR pbr;
    unpackGBuffer(pbr);

    sampleSSAO(pbr);

    color.rgb = ShadingPoint(pbr, draw_id);
    color.a = 1.0;
}
