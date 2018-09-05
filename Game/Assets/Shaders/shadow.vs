#define MAX_BONES 64

layout(location=0) in vec3 vertex;
#if defined(SKINNING)
layout(location=3) in ivec4 bone_indices;
layout(location=4) in vec4  bone_weights;
#endif

layout (std140) uniform light
{
	uniform mat4 projection;
	uniform mat4 view;
};

#if defined(SKINNING)
uniform mat4 palette[MAX_BONES];
#endif
#
uniform mat4 model;

void main()
{
#if defined(SKINNING)
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

	gl_Position = projection*view*model*skin_transform*vec4(vertex, 1);
#else
	gl_Position = projection*view*model*vec4(vertex, 1);
#endif
}

