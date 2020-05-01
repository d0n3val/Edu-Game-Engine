#define MAX_BONES 64

layout(location = 0) in vec3 vertex_position;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;

subroutine void TransformOutput();

layout(location=0) subroutine uniform TransformOutput transform_output;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat4 palette[MAX_BONES];

vec3 position;
out vec2 uv0;

layout(index=0) subroutine(TransformOutput) void transform_output_rigid()
{
    position = (model*vec4(vertex_position, 1.0)).xyz;
}

layout(index=1) subroutine(TransformOutput) void transform_output_skinning()
{
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

    position = (skin_transform*vec4(vertex_position, 1.0)).xyz;
}

void main()
{
    transform_output();

    gl_Position = proj*view*vec4(position, 1.0);
    uv0 = vertex_uv0;
}
