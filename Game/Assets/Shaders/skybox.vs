layout(location = 0) in vec3 position;

out vec3 coords;

layout (std140) uniform camera
{
	uniform mat4 projection;
	uniform mat4 view;
};

void main()
{
    coords      = position;
    vec4 pos    = projection*vec4(mat3(view)*position, 1.0); // not translation of view
    gl_Position = pos.xyww; // to ensure z = 1.0
}
