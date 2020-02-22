#define MAX_BONES 64
#define MAX_MORPH_TARGETS 128
#define CASCADE_COUNT 3

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;
layout(location = 5) in vec3 vertex_tangent;

subroutine void TransformOutput();

layout(location=0) subroutine uniform TransformOutput transform_output;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat4 palette[MAX_BONES];

uniform samplerBuffer morph;
uniform int morph_target_stride;
uniform int morph_normals_stride;
uniform int morph_tangents_stride;
uniform float morph_weights[MAX_MORPH_TARGETS];
uniform int num_morph_targets;

struct VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

out VertexOut fragment;

vec3 morph_targets_position(vec3 position)
{
    vec3 res = position;
    for(int i=0; i< num_morph_targets; ++i)
    {
        res += texelFetch(morph, morph_target_stride*i+gl_VertexID).xyz*morph_weights[i];
    }

    return res;
}

vec3 morph_targets_normal(vec3 normal)
{
    vec3 res = normal;
    for(int i=0; i< num_morph_targets; ++i)
    {
        res = normalize(res+texelFetch(morph, morph_target_stride*i+morph_normals_stride+gl_VertexID).xyz*morph_weights[i]);
    }

    return res;
}

vec3 morph_targets_tangent(vec3 tangent)
{
    vec3 res = tangent;
    for(int i=0; i< num_morph_targets; ++i)
    {
        res = normalize(res+texelFetch(morph, morph_target_stride*i+morph_tangents_stride+gl_VertexID).xyz*morph_weights[i]);
    }

    return res;
}


layout(index=0) subroutine(TransformOutput) void transform_output_rigid()
{
    fragment.position = (model*vec4(morph_targets_position(vertex_position), 1.0)).xyz;
    fragment.normal   = (model*vec4(morph_targets_normal(vertex_normal), 0.0)).xyz;
    fragment.tangent  = (model*vec4(morph_targets_tangent(vertex_tangent), 0.0)).xyz;
    fragment.uv0      = vertex_uv0;
}

layout(index=1) subroutine(TransformOutput) void transform_output_skinning()
{
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

    fragment.position = (skin_transform*vec4(morph_targets_position(vertex_position), 1.0)).xyz;
    fragment.normal   = (skin_transform*vec4(morph_targets_normal(vertex_normal), 0.0)).xyz;
    fragment.tangent  = (skin_transform*vec4(morph_targets_tangent(vertex_tangent), 0.0)).xyz;
    fragment.uv0      = vertex_uv0;
}

void main()
{
    transform_output();

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
