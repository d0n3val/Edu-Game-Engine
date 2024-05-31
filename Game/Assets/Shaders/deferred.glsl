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
uniform layout(location=DEFERRED_WIDTH_LOCATION) int width;
uniform layout(location=DEFERRED_HEIGHT_LOCATION) int height;

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

    vec4 positionSmp = texture(position, uv);
    pbr.position   = getWorldPos(texture(depth, uv).r, uv); 
    pbr.smoothness = specularSmp.a;
    pbr.occlusion  = emissiveSmp.a;
    pbr.planarReflections = albedoSmp.a != 0.0 ? 1 : 0;
}

void sampleSSAO(inout PBR pbr)
{
    pbr.occlusion *= texture(ssao, uv).r;
}

int getTileIndex()
{
    ivec2 pixel = ivec2(uv*vec2(width, height)/vec2(TILE_CULLING_GROUP_SIZE, TILE_CULLING_GROUP_SIZE));
    return TILE_INDEX(pixel.x, pixel.y, width);
}

void main()
{
    PBR pbr;
    unpackGBuffer(pbr);

    sampleSSAO(pbr);

    pbr.shadow = computeShadow(pbr.position);

    color = ShadingNoPoint(pbr, getTileIndex());

#if 0
    int bufferOffset = getTileIndex()*MAX_NUM_LIGHTS_PER_TILE;
    int lightIndex = texelFetch(spotLightList, int(bufferOffset)).r;
    if(lightIndex < 0)
        color.rgb = vec3(10000);
    else 
        color.rgb = vec3(0);
#endif 
}
