#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

layout(std430, row_major, binding = DRAWCOMMAND_SSBO_BINDING) buffer DrawCommands
{
    DrawCommand commands[];
};

layout(std430, binding = DISTANCES_SSBO_BINDING ) buffer Distances
{
    float cameraDistances[];
}

writeonly layout(std430, binding = COMAMNDCOUNT_SSBO_BINDING) buffer ParameterBuffer 
{
    int count;
};

void swap(inout DrawCommand a, inout DrawCommand b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

void swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

layout(local_size_x = FRUSTUM_CULLING_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
#ifdef ODD
    int index = int(gl_GlobalInvocationID.x)*2;
#else
    int index = int(gl_GlobalInvocationID.x)*2+1;
#endif

    if(index+1 < count)
    {
        if(cameraDistances[index] < cameraDistances[index+1])
        {
            swap(cameraDistances[index], cameraDistances[index+1]);
            swap(commands[index], commands[index+1]);
        }
    }
}