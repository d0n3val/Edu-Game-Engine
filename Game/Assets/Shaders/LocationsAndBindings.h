#ifndef _LOCATIONS_AND_BINDINGS_H_
#define _LOCATIONS_AND_BINDINGS_H_

// NOTE: This header is expected to be included from c++ and from shaders

// Vertex Attrib Locations
#define POSITION_ATTRIB_LOCATION 0
#define NORMAL_ATTRIB_LOCATION 1
#define UV0_ATTRIB_LOCATION 2
#define BONE_INDEX_ATTRIB_LOCATION 3
#define BONE_WEIGHT_ATTRIB_LOCATION 4
#define TANGENT_ATTRIB_LOCATION 5
#define UV1_ATTRIB_LOCATION 6
#define DRAW_ID_ATTRIB_LOCATION 7

// UBO
#define CAMERA_UBO_BINDING 0
#define SSAO_KERNEL_BINDING 1

// SSBO
#define MATERIAL_SSBO_BINDING 5
#define MODEL_SSBO_BINDING 10
#define PERINSTANCE_SSBO_BINDING 15
#define PALETTE_SSBO_BINDING 16
#define MORPH_WEIGHT_SSBO_BINDING 17

#define DIRLIGHT_SSBO_BINDING 12
#define POINTLIGHT_SSBO_BINDING 13
#define SPOTLIGHT_SSBO_BINDING 14
#define QUADLIGHT_SSBO_BINDING 15
#define SPHERELIGHT_SSBO_BINDING 16
#define TUBELIGHT_SSBO_BINDING 17
#define POINT_PROXY_POSITION_BINDING 1

// Texture Buffer Objects
#define MORPH_TARGET_TBO_BINDING 13

// Textures
#define MATERIAL_TEX_BINDING 0
#define DIFFUSE_IBL_TEX_BINDING 10
#define PREFILTERED_IBL_TEX_BINDING 11
#define ENVIRONMENT_BRDF_TEX_BINDING 12
#define AO_TEX_BINDING 13

#define GBUFFER_ALBEDO_TEX_BINDING 0 
#define GBUFFER_SPECULAR_TEX_BINDING 1
#define GBUFFER_EMISSIVE_TEX_BINDING 2
#define GBUFFER_POSITION_TEX_BINDING 3
#define GBUFFER_NORMAL_TEX_BINDING 4

#define DECAL_ALBEDO_TEX_BINDING 5
#define DECAL_NORMAL_TEX_BINDING 6
#define DECAL_SPECULAR_TEX_BINDING 7

#define SSAO_TEX_BINDING 8
#define SHADOWMAP_TEX_BINDING 24
#define VARIANCE_TEX_BINDING 25

#define SSAO_POSITIONS_BINDING 0
#define SSAO_NORMALS_BINDING 1

#define GAUSSIAN_BLUR_IMAGE_BINDING 0
#define GAUSSIAN_BLUR_INVIMAGE_SIZE_LOCATION 0

#define BLOOM_IMAGE_BINDING 0
#define BLOOM_DEPTH_BINDING 1
#define BLOOM_EMISSIVE_BINDING 2

#define FXAA_LDR_BINDING 0

#define KAWASE_INPUT_BINDING 0

#define SKINNING_PALETTE_BINDING 0
#define SKINNING_INDICES_BINDING 1
#define SKINNING_WEIGHTS_BINDING 2
#define SKINNING_POSITIONS_BINDING 3
#define SKINNING_INNORMALS_BINDING 4
#define SKINNING_INTANGENTS_BINDING 5
#define SKINNING_OUTPOSITIONS_BINDING 6
#define SKINNING_OUTNORMALS_BINDING 7
#define SKINNING_OUTTANGENTS_BINDING 8
#define SKINNING_MORPH_WEIGHTS_BINDING 9
#define SKINNING_MORPH_TARGET_BINDING 10

#define REDIMAGE_IMAGE_BINDING 0
#define REDIMAGE_GROUP_WIDTH 8
#define REDIMAGE_GROUP_HEIGHT 8
#define REDIMAGE_WIDHT_LOCATION 10
#define REDIMAGE_HEIGHT_LOCATION 11


#define SKINNING_NUM_VERTICES_LOCATION 20
#define SKINNING_NUM_TARGETS_LOCATION 21
#define SKINNING_TARGET_STRIDE_LOCATION 22
#define SKINNING_NORMAL_STRIDE_LOCATION 23
#define SKINNING_TANGENT_STRIDE_LOCATION 24
#define SKINNING_NUM_BONES_LOCATION 25

#define SKINNING_GROUP_SIZE 64


// Uniform Locations
#define FOG_DENSITY_HEIGHT_FALLOFF_LOCATION 0
#define FOG_GLOGAL_DENSITY_LOCATION 1
#define FOG_COLOR 2
#define FOG_SUN_COLOR 3

#define PREFILTERED_LOD_LEVELS_LOCATION 64
#define MODEL_LOCATION 65
#define INV_MODEL_LOCATION 66
#define NORMAL_STRENGTH_LOCATION 67

#define FXAA_SUBPIXELBLENDING_LOCATION 0

#define SHADOW_VIEWPROJ_LOCATION 20
#define SHADOW_BIAS_LOCATION 23
#define SHADOW_SLOPEBIAS_LOCATION 24

#define SSAO_SCREENSIZE_LOCATION 1
#define SSAO_RADIUS_LOCATION 2
#define SSAO_BIAS_LOCATION 3

// DEPTH PARALLEL REDUCTION
#define DEPTHREDUCTION_GROUP_WIDTH 8
#define DEPTHREDUCTION_GROUP_HEIGHT 4
#define DEPTH_INTEXTURE_BINDING 0 
#define DEPTH_OUTIMAGE_BINDING 1
#define DEPTH_WIDTH_LOCATION 0
#define DEPTH_HEIGHT_LOCATION 1

#endif /*_LOCATIONS_AND_BINDINGS_H_ */ 