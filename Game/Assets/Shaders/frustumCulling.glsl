#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

struct DrawInstance
{
    uint indexCount;
    uint baseIndex;
    uint baseVertex;
    uint baseInstance;
    vec4 sphere;
};

struct DrawCommand
{
    uint indexCount;
    uint instanceCount;
    uint baseIndex;
    uint baseVertex;
    uint baseInstance;
}; 

readonly layout(std430, row_major, binding = MODEL_SSBO_BINDING) buffer Transforms
{
    mat4 models[];
};

readonly layout(std430, binding = DRAWINSTAANCE_SSBO_BINDING) buffer Instances
{
    DrawInstance instances[];
};

writeonly layout(std430, row_major, binding = DRAWCOMMAND_SSBO_BINDING) buffer DrawCommands
{
    DrawCommand commands[];
};

writeonly layout(std430, binding = COMAMNDCOUNT_SSBO_BINDING) buffer ParameterBuffer 
{
    int count;
};

uniform layout(location=FRUSTUM_PLANES_LOCATION) vec4 planes[6];

uniform layout(location= NUM_INSTANCES_LOCATION) int numInstances;

#ifdef TRANSPARENTS

writeonly layout(std430, binding = DISTANCES_SSBO_BINDING) buffer Distances
{
    float cameraDistances[];
};

uniform layout(location = CAMERA_POS_LOCATION) vec3 cameraPos;

#endif 


layout(local_size_x = FRUSTUM_CULLING_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int index = int(gl_GlobalInvocationID.x);

    if(index < numInstances)
    {
        vec3 pos = instances[index].sphere.xyz+models[index][3].xyz;
        float radius = instances[index].sphere.w;

        if(dot(pos, planes[0].xyz) < radius &&
           dot(pos, planes[1].xyz) < radius &&
           dot(pos, planes[2].xyz) < radius &&
           dot(pos, planes[3].xyz) < radius)
        {

            int cmdIndex = atomicAdd(count, 1);
            
            commands[cmdIndex].indexCount    = instances[index].indexCount;
            commands[cmdIndex].instanceCount = 1;
            commands[cmdIndex].baseIndex     = instances[index].baseIndex;
            commands[cmdIndex].baseVertex    = instances[index].baseVertex;
            commands[cmdIndex].baseInstance  = index;

#ifdef TRANSPARENTS
            cameraDistances[cmdIndex] = distance(cameraPos, pos);
#endif 
        }
    }

}