layout(location = 0) in vec3 vertex_position;
layout(location = 2) in vec2 vertex_uv0;

void main()
{
    gl_Position = vec4(vertex_uv0*2-1, 0.0, 1.0);
}
	
