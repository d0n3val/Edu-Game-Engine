layout(location = 0) in vec3 vertex_position;

#if TEXCOORD1
layout(location = 6) in vec2 vertex_uv;
#else
layout(location = 2) in vec2 vertex_uv;
#endif

void main()
{
    gl_Position = vec4(mod(vertex_uv, 1.0f)*2-1, 0.0, 1.0);
}
	
