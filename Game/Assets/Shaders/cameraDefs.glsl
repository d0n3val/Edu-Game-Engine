#ifndef _CAMERA_DEFS_GLSL_
#define _CAMERA_DEFS_GLSL_

#include "/shaders/LocationsAndBindings.h"

layout(std140, row_major, binding = CAMERA_UBO_BINDING) uniform Camera 
{
    mat4 proj;
    mat4 view;
    mat4 invView;
    vec4 view_pos;
};


float getLinearZ(float depth)
{
    return -proj[3][2]/(proj[2][2]+depth*2.0-1.0);
}

vec3 getWorldPos(float depth, vec2 texCoords)
{
    float viewZ = getLinearZ(depth);
    float viewX = (texCoords.x*2.0-1.0)*(-viewZ)/proj[0][0];
    float viewY = (texCoords.y*2.0-1.0)*(-viewZ)/proj[1][1];
    vec3 viewPos = vec3(viewX, viewY, viewZ);
    return (invView*vec4(viewPos, 1.0)).xyz;
}


#endif /* _CAMERA_DEFS_GLSL_ */
