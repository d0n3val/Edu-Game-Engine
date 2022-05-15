#ifndef _SHADOWS_GLSL_
#define _SHADOWS_GLSL_

#define SHADOW_PCF

#include "/shaders/LocationsAndBindings.h"

#ifdef CASCADE_SHADOWMAP

#define CASCADE_COUNT 3

#if ENABLE_SOFT
layout(location=110) uniform sampler2D shadow_map[3];
#else
layout(location=110) uniform sampler2DShadow shadow_map[3];
#endif

layout(location=113) uniform float shadow_bias;
layout(location=114) uniform int kernel_half_size;

uniform vec2 map_size[CASCADE_COUNT];
in vec4 shadow_coord[3];

vec3 ComputeShadow(in vec3 color)
{
    for(uint i=0; i< 3; ++i)
    {
        if(shadow_coord[i].x >= 0.0 && shadow_coord[i].x < 1.0 && 
           shadow_coord[i].y >= 0.0 && shadow_coord[i].y < 1.0 && 
           shadow_coord[i].z >= 0.0 && shadow_coord[i].z < 1.0)
        {

#if ENABLE_SOFT

            vec2 moments = texture(shadow_map[i], shadow_coord[i].xy).rg;

            if(shadow_coord[i].z > moments.r+shadow_bias)
            {
                float variance = moments.g - (moments.r*moments.r);

                float d = moments.x - shadow_coord[i].z;
                float p_max = variance / (variance + d*d);

                color.rgb = color.rgb*(p_max/2+0.5);
            }


#else 

            // Compute PCF

            vec3 coord = vec3(shadow_coord[i].xy, shadow_coord[i].z-shadow_bias);

            float xOffset = 1.0/map_size[i].x;
            float yOffset = 1.0/map_size[i].y;

            float shadow_factor = 0.0;
            int total_samples = (kernel_half_size*2+1)*(kernel_half_size*2+1);

            for (int y = -kernel_half_size ; y <= kernel_half_size ; y++) 
            {
                for (int x = -kernel_half_size ; x <= kernel_half_size ; x++) 
                {
                    vec2 Offsets = vec2(x * xOffset, y * yOffset);
                    coord.xy = shadow_coord[i].xy + Offsets;
                    shadow_factor += texture(shadow_map[i], coord);
                }
            }

            shadow_factor /= float(total_samples);

            color.rgb = mix(vec3(0.0), color.rgb, 0.5+shadow_factor*0.5);
#endif


            break;
        }
    }

    return color;
}

#else 

#ifdef SHADOW_PCF
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2DShadow shadow_map;
#else
layout(binding=SHADOWMAP_TEX_BINDING) uniform sampler2D shadow_map;
#endif 
layout(location=SHADOW_BIAS_LOCATION) uniform float shadow_bias;

float ComputeShadow(in vec3 coord)
{
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