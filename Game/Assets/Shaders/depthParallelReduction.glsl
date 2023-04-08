#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

#define KERNEL_SIZE 32  

uniform layout(binding=DEPTH_INTEXTURE_BINDING) sampler2D inTexture;
uniform layout(binding=DEPTH_OUTIMAGE_BINDING, rg32f) writeonly image2D outImage; 
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

void main()
{
    // ensure work item is not out of work domain
    if(gl_GlobalInvocationID.x < (inWidth+(KERNEL_SIZE-1))/KERNEL_SIZE && gl_GlobalInvocationID.y < (inHeight+(KERNEL_SIZE-1))/KERNEL_SIZE)
    {
        ivec2 outCoord = ivec2(gl_GlobalInvocationID.xy);
        ivec2 inCoord  = outCoord*KERNEL_SIZE;
        ivec2 clampMax = ivec2(inWidth-1, inHeight-1);

        vec4 value = texelFetchOffset(inTexture, inCoord, 0, ivec2(0, 0));
        float minValue = GETMINCHANNEL(value);
        float maxValue = GETMAXCHANNEL(value);

        for(int x = 0;  x < KERNEL_SIZE; ++x)
        {
            for(int y=0; y < KERNEL_SIZE; ++y)
            {
                ivec2 coord = min(inCoord+ivec2(x, y), clampMax);
                value = texelFetch(inTexture, coord, 0);

                minValue = min(GETMINCHANNEL(value), minValue);

                // avoid 1.0 max (corresponding to skybox)
                float maxChannel = GETMAXCHANNEL(value);
                if(maxValue == 1.0) maxValue = maxChannel;
                if(maxChannel != 1.0) maxValue = max(GETMAXCHANNEL(value), maxValue);

                imageStore(outImage, outCoord, vec4(minValue, maxValue, 0.0, 0.0));
            }
        }
        
    }
}
