#ifndef _SHADOWS_GLSL_
#define _SHADOWS_GLSL_

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


#endif /* _SHADOWS_GLSL_ */