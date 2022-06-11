#ifndef _VERTEX_DEFS_GLSL_
#define _VERTEX_DEFS_GLSL_

#ifdef CASCADE_SHADOWMAP
#define NUM_CASCADES 3
#endif 

struct GeomData
{
    vec3 position;
    vec3 normal;
    vec3 tangent;
};

struct ShadowData
{
#ifdef CASCADE_SHADOWMAP
    vec3 shadowCoord[NUM_CASCADES];
#else 
    vec3 shadowCoord;
#endif 

};

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    GeomData geom;
#ifndef SHADOW_MAP
    ShadowData shadow;
#endif 
};

#endif /* _VERTEX_DEFS_GLSL_ */