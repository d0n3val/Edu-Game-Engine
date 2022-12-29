#ifndef _SHADOWS_GLSL_
#define _SHADOWS_GLSL_

#define SHADOW_PCF

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/vertexDefs.glsl"

#ifdef CASCADE_SHADOWMAP

#ifdef SHADOW_PCF
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2DShadow shadow_map[NUM_CASCADES];
#else
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2D shadow_map[NUM_CASCADES];
#endif

layout(location=SHADOW_VIEWPROJ_LOCATION) uniform mat4 shadowViewProj[NUM_CASCADES];

layout(location=SHADOW_BIAS_LOCATION) uniform float shadow_bias;

struct ShadowData
{
    vec3 shadowCoord[NUM_CASCADES];
};

float computeShadow(in vec3 position)
{
    for(uint i=0; i< NUM_CASCADES; ++i)
    {
        vec4 coord = shadowViewProj[i]*vec4(position, 1.0);
        coord.xyz /= coord.w;
        coord.xy = coord.xy*0.5+0.5;

        if(coord.x >= 0.0 && coord.x <= 1.0 && 
           coord.y >= 0.0 && coord.y <= 1.0 &&
           coord.z >= 0.0 && coord.z <= 1.0)
        {

#ifdef SHADOW_PCF
            ivec2 texSize = textureSize(shadow_map[i], 0);

            float xOffset = 1.0/float(texSize.x);
            float yOffset = 1.0/float(texSize.y);

            float shadow_factor = 0.0;

            for (int y = -1 ; y <= 1 ; y++) 
            {
                for (int x = -1 ; x <= 1; x++) 
                {
                    vec2 offsets = vec2(x * xOffset, y * yOffset);
                    shadow_factor += texture(shadow_map[i], vec3(coord.xy+offsets, coord.z-shadow_bias));
                }
            }

            return shadow_factor/9.0;
#else     
            float mapDepth = texture(shadow_map[i], coord.xy).r;
            if(coord.z > mapDepth+shadow_bias)
            {
                return 0.0;
            }

            return 1.0;
#endif 
            
        }
    }

    return 1.0;
}

#else  /* CASCADE */

struct ShadowData
{
    vec3 shadowCoord;
};

#ifdef SHADOW_PCF
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2DShadow shadow_map;
#else
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2D shadow_map;
#endif 
layout(location=SHADOW_BIAS_LOCATION) uniform float shadow_bias;

layout(location=SHADOW_VIEWPROJ_LOCATION) uniform mat4 shadowViewProj;

float computeShadow(in vec3 position)
{
    vec4 coord = shadowViewProj*vec4(position, 1.0);
    coord.xyz /= coord.w;
    coord.xy = coord.xy*0.5+0.5;


    if(coord.x >= 0.0 && coord.x <= 1.0 && 
       coord.y >= 0.0 && coord.y <= 1.0 &&
       coord.z >= 0.0 && coord.z <= 1.0)
    {
#ifdef SHADOW_PCF
        ivec2 texSize = textureSize(shadow_map, 0);

        float xOffset = 1.0/float(texSize.x);
        float yOffset = 1.0/float(texSize.y);

        float shadow_factor = 0.0;

        for (int y = -1 ; y <= 1 ; y++) 
        {
            for (int x = -1 ; x <= 1; x++) 
            {
                vec2 offsets = vec2(x * xOffset, y * yOffset);
                shadow_factor += texture(shadow_map, vec3(coord.xy+offsets, coord.z-shadow_bias));
            }
        }

        return shadow_factor/9.0;
#else     
        float mapDepth = texture(shadow_map, coord.xy).r;
        if(coord.z > mapDepth+shadow_bias)
        {
            return 0.0;
        }

#endif 
    }

    return 1.0;
}

#endif  // CASCADE_SHADOWMAP

#endif /* _SHADOWS_GLSL_ */