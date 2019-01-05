layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv0;

out vec2 uv;

void main()
{
    gl_Position = vec4(vertex_position, 1.0);
    uv = vertex_uv0;
}
