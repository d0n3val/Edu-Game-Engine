#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"

layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;
uniform layout(location=TILECULLING_WIDTH_LOCATION) int width;
uniform layout(location=TILECULLING_HEIGHT_LOCATION) int height;
uniform layout(location=TILECULLING_MAX_POINT_LIGHTS_LOCATION ) int maxPointLights;
uniform layout(location=TILECULLING_MAX_SPOT_LIGHTS_LOCATION ) int maxSpotLights;

shared uint minZ;
shared uint maxZ;
shared uint pointsInTile;
shared uint spotsInTile;
shared uint volSpotsInTile;

uniform layout(binding=TILE_CULLING_POINTIMAGE_BINDING, r32i) writeonly iimageBuffer pointList;
uniform layout(binding=TILE_CULLING_SPOTIMAGE_BINDING, r32i) writeonly iimageBuffer spotList;
uniform layout(binding=TILE_CULLING_VOLSPOTIMAGE_BINDING, r32i) writeonly iimageBuffer volSpotList;
//uniform layout(binding=TILE_CULLING_DBGIMAGE_BINDING, rgba32f) imageBuffer dbgImage;


// Note: Plane is created from 3 points but we assume the other point is (0, 0) because we are in view space
vec4 createPlaneEq(in vec3 p0, in vec3 p1)
{
    vec3 N = normalize(cross(p0, p1));

    return vec4(N, 0);
}

layout(local_size_x = TILE_CULLING_GROUP_SIZE, local_size_y = TILE_CULLING_GROUP_SIZE, local_size_z = 1) in;
void main()
{
    uvec2 globalIdx = gl_GlobalInvocationID.xy;
    if(globalIdx.x < width && globalIdx.y < height)
    {
        vec2 uv = vec2(globalIdx)/vec2(width, height);

        float d = texture(depth, uv).r;

        if(gl_LocalInvocationIndex == 0)
        {
            minZ = 0xFFFFFFFF;
            maxZ = 0;
            pointsInTile = 0;
            spotsInTile = 0;
            volSpotsInTile = 0;
        }

        barrier(); // ensure all barriers arrived to here (memory barrier is not needed)

        atomicMin(minZ, floatBitsToUint(d));
        atomicMax(maxZ, floatBitsToUint(d));

        barrier(); // ensure all barriers arrived to here (memory barrier is not needed)


        float viewMinZ    = getLinearZ(uintBitsToFloat(minZ));
        float volViewMinZ = getLinearZ(0.0);
        float viewMaxZ    = getLinearZ(uintBitsToFloat(maxZ));

        // Compute frustum planes    

        uint numTilesX = NUM_TILES(width);
        uint numTilesY = NUM_TILES(height);

        vec3 planePoints[4];
        planePoints[0] = getViewPos(1.0, vec2(float(gl_WorkGroupID.x)/float(numTilesX),   float(gl_WorkGroupID.y)/float(numTilesY)));
        planePoints[1] = getViewPos(1.0, vec2(float(gl_WorkGroupID.x+1)/float(numTilesX), float(gl_WorkGroupID.y)/float(numTilesY)));
        planePoints[2] = getViewPos(1.0, vec2(float(gl_WorkGroupID.x+1)/float(numTilesX), float(gl_WorkGroupID.y+1)/float(numTilesY)));
        planePoints[3] = getViewPos(1.0, vec2(float(gl_WorkGroupID.x)/float(numTilesX),   float(gl_WorkGroupID.y+1)/float(numTilesY)));

        vec4 planes[4];

        for(int i=0; i< 4; ++i)
        {
            planes[i] = createPlaneEq(planePoints[i], planePoints[(i+1)&3]);
        }

        // Do Frustum Culling

        const uint threadCount = gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z;
        int tileIndex = int(TILE_INDEX(gl_WorkGroupID.x, gl_WorkGroupID.y, width));
        uint threadIndex = gl_LocalInvocationIndex;

        // Point lights
        for(uint i=threadIndex;i<num_point; i+=threadCount)
        {
            PointLight light = points[i];
            float radius  = light.position.w;
            vec3 viewPos = (view*vec4(light.position.xyz, 1.0)).xyz;
    
            if(dot(viewPos, planes[0].xyz) < radius &&
               dot(viewPos, planes[1].xyz) < radius &&
               dot(viewPos, planes[2].xyz) < radius &&
               dot(viewPos, planes[3].xyz) < radius &&
               (viewMinZ-viewPos.z) > -radius &&
               (viewMaxZ-viewPos.z) < radius)
            {
                uint index = atomicAdd(pointsInTile, 1);

                if(index < maxPointLights)
                {
                    imageStore(pointList, int(tileIndex*maxPointLights+index), ivec4(i));
                }
            }

        }

        bool inside = false;

        for(uint i=threadIndex;i<num_spot; i+=threadCount)
        {
            SpotLight light = spots[i];
            vec4 position   = getSpotLightSphere(light);
            float radius    = position.w;
            vec3 viewPos    = (view*vec4(position.xyz, 1.0)).xyz;
    
            if(dot(viewPos, planes[0].xyz) < radius &&
               dot(viewPos, planes[1].xyz) < radius &&
               dot(viewPos, planes[2].xyz) < radius &&
               dot(viewPos, planes[3].xyz) < radius &&
              (viewMaxZ-viewPos.z) < radius) 
            {

                if((viewMinZ-viewPos.z) > -radius)
                {
                    uint index = atomicAdd(spotsInTile, 1);

                    if(index < maxSpotLights)
                    {
                        imageStore(spotList, int(tileIndex*maxSpotLights+index), ivec4(i));
                    }
                }

                if ((volViewMinZ-viewPos.z) > -radius)
                {
                    uint index = atomicAdd(volSpotsInTile, 1);

                    if(index < maxSpotLights)
                    {
                        imageStore(volSpotList, int(tileIndex*maxSpotLights+index), ivec4(i));
                        inside = true;
                    }

                }

            }

            //imageStore(dbgImage, tileIndex, vec4(viewMaxZ, viewPos.z, globalIdx.x, inside));
        }

        barrier();

        // Mark last with a -1
        if(gl_LocalInvocationIndex == 0)
        {
            if(pointsInTile < maxPointLights) imageStore(pointList, int(tileIndex*maxPointLights+pointsInTile), ivec4(-1));
            if(spotsInTile < maxSpotLights) imageStore(spotList, int(tileIndex*maxSpotLights+spotsInTile), ivec4(-1));
            if(volSpotsInTile < maxSpotLights) imageStore(volSpotList, int(tileIndex*maxSpotLights+volSpotsInTile), ivec4(-1));
        }
    }
}