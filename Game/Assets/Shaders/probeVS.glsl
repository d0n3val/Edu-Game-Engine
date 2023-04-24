#version 460

layout(location = 0) in vec3 vertex_position;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec3 coords;
out vec3 position;

void main()
{
    coords = vertex_position;
    position = (model*vec4(vertex_position, 1.0)).xyz;
    gl_Position = proj*view*model*vec4(vertex_position, 1.0);
}
