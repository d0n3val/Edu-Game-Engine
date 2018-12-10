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

    AMBIENT_CONSTANT_LOC,
    DIFFUSE_CONSTANT_LOC,
    SPECULAR_CONSTANT_LOC

    AMBIENT_COLOR_LOC = 20,

    DIRECTIONAL_DIR_LOC,
    DIRECTIONAL_COLOR_LOC,

    POINT0_POSITION_LOC,
    POINT0_COLOR_LOC,
    POINT0_CONSTANT_ATT_LOC,
    POINT0_LINEAR_ATT_LOC,
    POINT0_QUADRIC_ATT_LOC,

    POINT1_POSITION_LOC,
    POINT1_COLOR_LOC,
    POINT1_CONSTANT_ATT_LOC,
    POINT1_LINEAR_ATT_LOC,
    POINT1_QUADRIC_ATT_LOC,

    POINT2_POSITION_LOC,
    POINT2_COLOR_LOC,
    POINT2_CONSTANT_ATT_LOC,
    POINT2_LINEAR_ATT_LOC,
    POINT2_QUADRIC_ATT_LOC,

    POINT3_POSITION_LOC,
    POINT3_COLOR_LOC,
    POINT3_CONSTANT_ATT_LOC,
    POINT3_LINEAR_ATT_LOC,
    POINT3_QUADRIC_ATT_LOC,

    NUM_POINT_LIGHT_LOC,

    SPOT0_POSITION_LOC,
    SPOT0_DIRECTION_LOC,
    SPOT0_COLOR_LOC,
    SPOT0_INNER_LOC,
    SPOT0_OUTTER_LOC,
    SPOT0_CONSTANT_ATT_LOC,
    SPOT0_LINEAR_ATT_LOC,
    SPOT0_QUADRIC_ATT_LOC,

    SPOT1_POSITION_LOC,
    SPOT1_DIRECTION_LOC,
    SPOT1_COLOR_LOC,
    SPOT1_INNER_LOC,
    SPOT1_OUTTER_LOC,
    SPOT1_CONSTANT_ATT_LOC,
    SPOT1_LINEAR_ATT_LOC,
    SPOT1_QUADRIC_ATT_LOC,

    SPOT2_POSITION_LOC,
    SPOT2_DIRECTION_LOC,
    SPOT2_COLOR_LOC,
    SPOT2_INNER_LOC,
    SPOT2_OUTTER_LOC,
    SPOT2_CONSTANT_ATT_LOC,
    SPOT2_LINEAR_ATT_LOC,
    SPOT2_QUADRIC_ATT_LOC,

    SPOT3_POSITION_LOC,
    SPOT3_DIRECTION_LOC,
    SPOT3_COLOR_LOC,
    SPOT3_INNER_LOC,
    SPOT3_OUTTER_LOC,
    SPOT3_CONSTANT_ATT_LOC,
    SPOT3_LINEAR_ATT_LOC,
    SPOT3_QUADRIC_ATT_LOC,

    NUM_SPOT_LIGHT_LOC
};

#endif /* __DEFAULT_LOCATIONS_H__ */
