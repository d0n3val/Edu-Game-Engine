#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"
#include "/shaders/shadows.glsl"

layout(binding = GBUFFER_ALBEDO_TEX_BINDING) uniform sampler2D albedo;
layout(binding = GBUFFER_SPECULAR_TEX_BINDING) uniform sampler2D specular;
layout(binding = GBUFFER_EMISSIVE_TEX_BINDING) uniform sampler2D emissive;
layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;
layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;
layout(binding = GBUFFER_NORMAL_TEX_BINDING) uniform sampler2D normal;
layout(binding = SSAO_TEX_BINDING) uniform sampler2D ssao;

in vec2 uv;
out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

void unpackGBuffer(out PBR pbr)
{
    vec4 albedoSmp   = texture(albedo, uv);
    vec4 specularSmp = texture(specular, uv);
    vec4 emissiveSmp = texture(emissive, uv);
    vec4 normalSmp   = texture(normal, uv);
    vec4 depthSmp    = texture(depth, uv);

    pbr.diffuse    = albedoSmp.rgb;
    pbr.shadow     = 1.0;
    pbr.specular   = specularSmp.rgb;
    pbr.emissive   = emissiveSmp.rgb;
    pbr.normal     = normalize(normalSmp.rgb*2.0-1.0);

#ifdef GEN_WORLD_POSITION
    float viewZ = -proj[3][2]/(proj[2][2]+texture(depth, uv).r);
    float viewX = (uv.x*2.0-1.0)*(-viewZ)/proj[0][0];
    float viewY = (uv.y*2.0-1.0)*(-viewZ)/proj[1][1];
    vec3 viewPos = vec3(viewX, viewY, viewZ);
    pbr.position = (invView*vec4(viewPos, 1.0)).xyz;
#else
    vec4 positionSmp = texture(position, uv);
    pbr.position   = positionSmp.rgb;
#endif 
    pbr.smoothness = specularSmp.a;
    pbr.occlusion  = emissiveSmp.a;
    pbr.planarReflections = albedoSmp.a != 0.0 ? 1 : 0;
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

    pbr.shadow = computeShadow(pbr.position);
    //pbr.shadow = 1.0;

    color = ShadingNoPoint(pbr);
}
