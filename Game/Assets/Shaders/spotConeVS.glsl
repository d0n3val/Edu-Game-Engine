#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;
layout(location = MODEL_MATRIX_BINDING) uniform mat4 model;

out vec4 clippingPos;
out vec4 viewPos;

void main()
{
    viewPos = view*model*vec4(vertex_position, 1.0);
    clippingPos = proj*viewPos;
    gl_Position = clippingPos;
}

