
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

struct VertexOut
{
    vec2 uv0;
    vec3 position;
};

out VertexOut fragment;


void main()
{
    fragment.position = (model*vec4(vertex_position, 1.0)).xyz;
    fragment.uv0      = vertex_uv0;

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
