layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 position;
} v_out;

void main()
{
    v_out.position = (model*vec4(vertex_position, 1.0)).xyz;
    v_out.normal   = (model*vec4(vertex_normal, 0.0)).xyz;
    v_out.uv0      = vertex_uv0;

    gl_Position = proj*view*vec4(v_out.position, 1.0);
}
