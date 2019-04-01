
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_color;
layout(location = 2) in vec2 vertex_uv0;

uniform mat4 proj;
uniform mat4 view;

struct VertexOut
{
    vec2 uv;
    vec4 color;
};

out VertexOut fragment;

void main()
{
    gl_Position = proj*view*vec4(vertex_position, 1.0);
    fragment.color = vertex_color;
    fragment.uv = vertex_uv0;
}
