#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

uniform layout(binding=DEPTH_INTEXTURE_BINDING) sampler2D inTexture;
uniform layout(binding=DEPTH_OUTIMAGE_BINDING, rg32f) restrict writeonly image2D outImage; 
uniform layout(location=DEPTH_WIDTH_LOCATION) int inWidth;
uniform layout(location=DEPTH_HEIGHT_LOCATION) int inHeight;

layout(local_size_x = DEPTHREDUCTION_GROUP_WIDTH, local_size_y = DEPTHREDUCTION_GROUP_HEIGHT, local_size_z = 1) in;

#ifdef USE_RED_CHANNEL
#define GETMINCHANNEL(color) color.r
#define GETMAXCHANNEL(color) color.r
#else
#define GETMINCHANNEL(color) color.r
#define GETMAXCHANNEL(color) color.g
#endif 

shared vec2 values[gl_WorkGroupSize.x*gl_WorkGroupSize.y];

void main()
{
    // ensure work item is not out of work domain
    if(gl_GlobalInvocationID.x < inWidth && gl_GlobalInvocationID.y < inHeight)
    {
        ivec2 inCoord = ivec2(gl_GlobalInvocationID.xy);
        vec4 value    = texelFetch(inTexture, inCoord, 0);

        float maxValue = GETMAXCHANNEL(value);
        if(maxValue == 1.0) maxValue = 0.0;

        values[gl_LocalInvocationIndex] = vec2(GETMINCHANNEL(value), maxValue);
    }
    else
    {
        values[gl_LocalInvocationIndex] = vec2(1.0, 0.0);
    }

    memoryBarrierShared(); // ensure memory is visible
    barrier(); // ensure all barriers arrived to here

    if(gl_LocalInvocationIndex == 0)
    {
        float minValue = 1.0;
        float maxValue = 0.0;

        int totalSize = int(gl_WorkGroupSize.x*gl_WorkGroupSize.y);
        for(int i=0; i< totalSize; ++i)
        {
            minValue = min(minValue, values[i].x);
            maxValue = max(maxValue, values[i].y);
        }

        imageStore(outImage, ivec2(gl_WorkGroupID.xy), vec4(minValue, maxValue, 0.0, 0.0));
    }
}
