#define MAX_BONES 64

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4  bone_weights;
layout(location = 5) in vec3 vertex_tangent;

subroutine void TransformOutput();

layout(location=0) subroutine uniform TransformOutput transform_output;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat4 palette[MAX_BONES];

struct VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

out VertexOut fragment;


layout(index=0) subroutine(TransformOutput) void transform_output_rigid()
{
    fragment.position = (model*vec4(vertex_position, 1.0)).xyz;
    fragment.normal   = (model*vec4(vertex_normal, 0.0)).xyz;
    fragment.tangent  = (model*vec4(vertex_tangent, 0.0)).xyz;
    fragment.uv0      = vertex_uv0;
}

layout(index=1) subroutine(TransformOutput) void transform_output_skinning()
{
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

    fragment.position = (skin_transform*vec4(vertex_position, 1.0)).xyz;
    fragment.normal   = (skin_transform*vec4(vertex_normal, 0.0)).xyz;
    fragment.tangent  = (skin_transform*vec4(vertex_tangent, 0.0)).xyz;
    fragment.uv0      = vertex_uv0;
}

void main()
{
    transform_output();

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
