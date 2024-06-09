#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

writeonly layout(std430, binding = INDIRECT_SSBO_BINDING) buffer IndirectBuffer 
{
    uint groupX;
    uint groupY;
    uint groupZ;
};

readonly layout(std430, binding = COMAMNDCOUNT_SSBO_BINDING) buffer ParameterBuffer 
{
    int count;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    groupX = (count+(FRUSTUM_CULLING_GROUP_SIZE-1))/FRUSTUM_CULLING_GROUP_SIZE;
    groupY = 1;
    groupZ = 1;
}