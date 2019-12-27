#ifndef __DEFAULT_LOCATIONS_H__
#define __DEFAULT_LOCATIONS_H__

#define MAX_NUM_POINT_LIGHTS 4
#define MAX_NUM_SPOT_LIGHTS 4

enum DefaultShaderLocations
{
    DIFFUSE_MAP_LOC = 0,
    DIFFUSE_COLOR_LOC,

    SPECULAR_MAP_LOC,
    SPECULAR_COLOR_LOC,
    SHININESS_LOC,

    OCCLUSION_MAP_LOC,

    EMISSIVE_MAP_LOC,
    EMISSIVE_COLOR_LOC,

    NORMAL_MAP_LOC,

    AMBIENT_COLOR_LOC = 20,

    DIRECTIONAL_DIR_LOC,
    DIRECTIONAL_COLOR_LOC,

    POINT0_POSITION_LOC,
    POINT0_COLOR_LOC,
    POINT0_ATTENUATION_LOC,

    POINT1_POSITION_LOC,
    POINT1_COLOR_LOC,
    POINT1_ATTENUATION_LOC,

    POINT2_POSITION_LOC,
    POINT2_COLOR_LOC,
    POINT2_ATTENUATION_LOC,

    POINT3_POSITION_LOC,
    POINT3_COLOR_LOC,
    POINT3_ATTENUATION_LOC,

    NUM_POINT_LIGHT_LOC,

    SPOT0_POSITION_LOC,
    SPOT0_DIRECTION_LOC,
    SPOT0_COLOR_LOC,
    SPOT0_ATTENUATION_LOC,
    SPOT0_INNER_LOC,
    SPOT0_OUTTER_LOC,

    SPOT1_POSITION_LOC,
    SPOT1_DIRECTION_LOC,
    SPOT1_COLOR_LOC,
    SPOT1_ATTENUATION_LOC,
    SPOT1_INNER_LOC,
    SPOT1_OUTTER_LOC,

    SPOT2_POSITION_LOC,
    SPOT2_DIRECTION_LOC,
    SPOT2_COLOR_LOC,
    SPOT2_ATTENUATION_LOC,
    SPOT2_INNER_LOC,
    SPOT2_OUTTER_LOC,

    SPOT3_POSITION_LOC,
    SPOT3_DIRECTION_LOC,
    SPOT3_COLOR_LOC,
    SPOT3_ATTENUATION_LOC,
    SPOT3_INNER_LOC,
    SPOT3_OUTTER_LOC,

    NUM_SPOT_LIGHT_LOC
};

enum DefaultFragmentSubroutineLocations
{
    GET_NORMAL_LOCATION = 0,
    GET_FRESNEL_LOCATION,
    NUM_FRAGMENT_SUBROUTINE_UNIFORMS
};

enum DefaultFragmentSubroutineIndices
{
    GET_NORMAL_FROM_VERTEX = 0,
    GET_NORMAL_FROM_TEXTURE,
    GET_FRESNEL_SCHLICK,
    GET_NO_FRESNEL
};

enum DefaultVertexSubroutineLocations
{
    TRANSFORM_OUTPUT = 0,
    NUM_VERTEX_SUBROUTINE_UNIFORMS
};

enum DefaultVertexSubroutineIndices
{
    TRANSFORM_OUTPUT_RIGID = 0,
    TRANSFORM_OUTPUT_SKINNING
};

#endif /* __DEFAULT_LOCATIONS_H__ */
