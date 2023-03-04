#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

uniform layout(binding=REDIMAGE_IMAGE_BINDING, rgba8) writeonly image2D myImage; 
uniform layout(location=REDIMAGE_WIDHT_LOCATION) int width;
uniform layout(location=REDIMAGE_HEIGHT_LOCATION) int height;

layout(local_size_x = REDIMAGE_GROUP_WIDTH, local_size_y = REDIMAGE_GROUP_HEIGHT, local_size_z = 1) in;

void main()
{
    // ensure work item is not out of work domain
    if(gl_GlobalInvocationID.x < width && gl_GlobalInvocationID.y < height)
    {
        imageStore(myImage, ivec2(gl_GlobalInvocationID.xy), vec4(1.0, 0.0, 0.0, 1.0));
    }
}
