#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;
layout(location = NORMAL_ATTRIB_LOCATION) in vec3 vertex_normal;
layout(location = UV0_ATTRIB_LOCATION) in vec2 vertex_texcoord;

layout(location = MODEL_MATRIX_BINDING) uniform mat4 model;

out vec4 clippingPos;
out vec3 worldNormal;
out vec3 worldPos;
out vec2 texcoord;

void main()
{
    mat3 normalMat = transpose(inverse(mat3(model)));
    worldNormal    = normalMat*vertex_normal;
    worldPos       = (model*vec4(vertex_position, 1.0)).xyz;
    texcoord       = vertex_texcoord;

    clippingPos = proj*view*model*vec4(vertex_position, 1.0);

    gl_Position = clippingPos;
}

