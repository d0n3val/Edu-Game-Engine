#version 440
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

#define KERNEL_SIZE 128
#define RANDOM_ROWS 4
#define RANDOM_COLS 4

out vec4 result;
in vec2 uv;

uniform sampler2D positions;
uniform sampler2D normals;

uniform vec2      screenSize;
uniform float     radius;
uniform float     bias;

layout(std140,binding = SSAO_KERNEL_BINDING) uniform Kernel
{
    vec4 offsets[KERNEL_SIZE];
    vec4 rots[RANDOM_ROWS][RANDOM_COLS];

} kernel;

mat3 createTangentSpace(const vec3 normal, const vec3 up)
{
    vec3 newUp = normalize(up-normal*dot(normal, up));
    vec3 right = normalize(cross(newUp, normal)); 
    return mat3(right, newUp, normal);
}

float getSampleDepth(in vec3 samplePos)
{
    vec4 clippingSpace = proj*vec4(samplePos, 1.0);
    vec2 sampleUV = (clippingSpace.xy/clippingSpace.w)*0.5+0.5;

    return (view*vec4(texture(positions, sampleUV).xyz, 1.0)).z;
}

void main()
{
    vec3 position = (view*vec4(texture(positions, uv).xyz, 1.0)).xyz;
    vec3 normal   = mat3(view)*normalize(texture(normals, uv).xyz*2.0-1.0);

    vec2 screenPos    = uv*screenSize;
    ivec2 rotIndex    = ivec2(int(mod(screenPos.y, RANDOM_ROWS)), int(mod(screenPos.x, RANDOM_COLS)));
    mat3 tangentSpace = createTangentSpace(normal, kernel.rots[rotIndex.x][rotIndex.y].xyz);

    int occlusion     = 0;

    for(int i=0; i< KERNEL_SIZE; ++i) 
    {
        vec3 offset       = tangentSpace*kernel.offsets[i].xyz; 
        vec3 samplePos    = position+offset*radius;

        float sampleDepth = getSampleDepth(samplePos);

        if(sampleDepth+bias >= samplePos.z && abs(sampleDepth-position.z) < radius)
        {
            ++occlusion;
        }
    }

#if 0
    vec3 offset       = tangentSpace*vec3(0.0, 0.0, 0.0); //kernel.offsets[i].xyz; 
    vec3 samplePos    = position; //+offset; //*radius;
    float sampleDepth = getSampleDepth(samplePos);
    vec4 clippingSpace = proj*vec4(samplePos, 1.0);
    vec2 sampleUV = (clippingSpace.xy/clippingSpace.w)*0.5+0.5;
    sampleDepth = (view*vec4(texture(positions, sampleUV).xyz, 1.0)).z;
    vec3 tmp = (view*vec4(texture(positions, sampleUV).xyz, 1.0)).xyz;
    result.xyz = position.zzz; //tmp.zzz;
#endif 
    
    result = vec4(vec3(1.0-float(occlusion)/float(KERNEL_SIZE)), 1.0f);
}