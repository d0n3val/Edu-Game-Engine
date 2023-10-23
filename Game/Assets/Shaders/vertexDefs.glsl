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
    float sign;
};

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    GeomData geom;
};

#endif /* _VERTEX_DEFS_GLSL_ */