#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

layout(binding = SPOTCONE_DEPTH_BINDING) uniform sampler2D depth;

out vec4 outColour;

in vec4 clippingPos;
in vec4 viewPos;

float linearizeDepth(float d)
{
    float S = proj[2].z;
    float T = proj[3].z;
    float U = proj[2].w;

    //d = d*2-1;

    return T/(S-d*U);
    //return T/(S+d);
}

void main()
{
    vec2 screenPos = clippingPos.xy/clippingPos.w;
    float coneDepth = clippingPos.z/clippingPos.w;
    vec2 screenUV = screenPos*0.5+0.5;

    float depthValue = texture(depth, screenUV).r;

    if(depthValue < coneDepth)
        discard;


    float alphaMult = linearizeDepth(depthValue)-linearizeDepth(coneDepth);
    alphaMult = alphaMult*alphaMult;
    alphaMult = clamp(alphaMult, 0.0, 1.0);

    //outColour = vec4(1.0, 1.0, 1.0, 0.5*alphaMult);
    outColour = vec4(1.0, 1.0, 1.0, 0.5*alphaMult);
}
