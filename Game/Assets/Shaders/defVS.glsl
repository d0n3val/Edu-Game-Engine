#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/vertexDefs.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;
layout(location = NORMAL_ATTRIB_LOCATION) in vec3 vertex_normal;
layout(location = UV0_ATTRIB_LOCATION) in vec2 vertex_uv0;
layout(location = TANGENT_ATTRIB_LOCATION) in vec4 vertex_tangent;
layout(location = UV1_ATTRIB_LOCATION) in vec2 vertex_uv1;

struct PerInstance
{
    uint indexCount;
    uint baseIndex;
    uint baseVertex;
    uint numBones;
    vec4 obb[8];
};

readonly layout(std430, row_major, binding = MODEL_SSBO_BINDING) buffer Transforms
{
    mat4 models[];
};

readonly layout(std430, binding = DRAWINSTAANCE_SSBO_BINDING) buffer Instances
{
    PerInstance instances[];
};


out VertexOut fragment;
out flat int draw_id;

void TransformOutput(out GeomData geom);

void main()
{
    GeomData geom;
    TransformOutput(geom);

    gl_Position = proj*view*vec4(geom.position, 1.0);

    fragment.geom = geom;
    fragment.uv0  = vertex_uv0;
    fragment.uv1  = vertex_uv1;
    draw_id       = gl_BaseInstance;
}


void TransformOutput(out GeomData geom)
{
    if(instances[gl_BaseInstance].numBones == 0)
    {
        mat4 model = models[gl_BaseInstance];
        mat3 normalMat = transpose(inverse(mat3(model)));

        geom.position = (model*vec4(vertex_position, 1.0)).xyz;

        geom.normal = normalize(normalMat*vertex_normal);
        geom.tangent = normalize(normalMat*vertex_tangent.xyz);
    }
    else
    {
        geom.position = vertex_position;

        geom.normal = vertex_normal;
        geom.tangent = vertex_tangent.xyz;
    }

    geom.bitangent = cross(geom.normal, geom.tangent) * vertex_tangent.w;
}
