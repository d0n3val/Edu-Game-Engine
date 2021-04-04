#version 440

layout(location = 0) in vec3 position;

out vec3 coords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    coords      = position;
    vec4 pos    = proj*vec4(mat3(view)*position, 1.0); // not translation of view
    gl_Position = pos.xyww; 
}
