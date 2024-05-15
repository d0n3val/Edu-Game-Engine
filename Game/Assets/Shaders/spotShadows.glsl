#ifndef _SPOT_SHADOWS_GLSL_
#define _SPOT_SHADOWS_GLSL_


#include "/shaders/LocationsAndBindings.h"
#include "/shaders/vertexDefs.glsl"


/*
layout(binding=SPOT_VARIANCE_TEX_BINDING) uniform sampler2D varianceMap[MAX_NUM_SPOT_SHADOWS];
layout(location=SPOT_SHADOW_BIAS_LOCATION) uniform float spotShadowBias[MAX_NUM_SPOT_SHADOWS];
layout(location=SPOT_SHADOW_VIEWPROJ_LOCATION) uniform mat4 spotShadowViewProj[MAX_NUM_SPOT_SHADOWS];
*/

float computeSpotShadow(sampler2D shadowMap, in mat4 viewProj, in vec3 position)
{
    vec4 coord = viewProj*vec4(position, 1.0);
    coord.xyz /= coord.w;
    coord.xy = coord.xy*0.5+0.5;

    if(coord.x >= 0.0 && coord.x <= 1.0 && 
       coord.y >= 0.0 && coord.y <= 1.0 &&
       coord.z >= 0.0 && coord.z <= 1.0)
    {
        vec2 moments = texture(shadowMap, coord.xy).rg;

        if(coord.z > moments.r)
        {
            float variance = max(moments.g - (moments.r*moments.r), 0.00002);
            float d = coord.z - moments.r;
            float p_max = variance / (variance + d*d); // factor used to interpolate between ambient and full litted 

            return p_max;
        }

        return 1.0;

#if 0
        float mapDepth = texture(shadowMap, coord.xy).r;
        if(coord.z > mapDepth+0.001/*+shadowMap*/)
        {
            return 0.0;
        }
#endif 

    }

    return 0.0;
}

#endif /* _SHADOWS_GLSL_ */
