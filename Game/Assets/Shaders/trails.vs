
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    gl_Position = proj*view*vec4(vertex_position, 1.0);
}
