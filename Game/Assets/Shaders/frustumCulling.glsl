#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

struct PerInstance
{
    uint indexCount;
    uint baseIndex;
    uint baseVertex;
    uint numBones;
    vec4 obb[8];
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
    PerInstance instances[];
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

uniform layout(location = CAMERA_POS_LOCATION) vec3 cameraPos;

#ifdef TRANSPARENTS

writeonly layout(std430, binding = DISTANCES_SSBO_BINDING) buffer Distances
{
    float cameraDistances[];
};

#endif 

bool inFrustum(in vec3 points[8])
{
    int isOut;
    for (int i = 0; i < 6; ++i)
    {
        isOut = 0;
        for (int k = 0; k < 8; ++k)
        {
            if(dot(planes[i].xyz, points[k])-planes[i].w  >= 0.0f )
                isOut++;
        }

        if (isOut == 8)
            return false;
    }

    return true;
}



layout(local_size_x = FRUSTUM_CULLING_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int index = int(gl_GlobalInvocationID.x);

    if(index < numInstances)
    {
        mat4 transform = models[index];

        vec3 points[8];
        for(uint i=0; i< 8; ++i) 
        {
            points[i] = (transform*instances[index].obb[i]).xyz;
        }

        if(inFrustum(points))
        {
            int cmdIndex = atomicAdd(count, 1);
            
            commands[cmdIndex].indexCount    = instances[index].indexCount;
            commands[cmdIndex].instanceCount = 1;
            commands[cmdIndex].baseIndex     = instances[index].baseIndex;
            commands[cmdIndex].baseVertex    = instances[index].baseVertex;
            commands[cmdIndex].baseInstance  = index;
        }
    }

}