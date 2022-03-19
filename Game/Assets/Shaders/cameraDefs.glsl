#ifndef _CAMERA_DEFS_GLSL_
#define _CAMERA_DEFS_GLSL_

#include "/shaders/LocationsAndBindings.h"

layout(std140, row_major, binding = CAMERA_UBO_BINDING) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec4 view_pos;
};



#endif /* _CAMERA_DEFS_GLSL_ */
