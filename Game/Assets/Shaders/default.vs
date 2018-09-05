#define MAX_BONES 64

// ATTRIBUTES 

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord0;

#if defined(SKINNING)
layout(location=3) in ivec4 bone_indices;
layout(location=4) in vec4  bone_weights;
#endif
#if NORMAL_MAP
layout(location=5) in vec3 tangent;
#endif

// UNIFORMS

layout (std140) uniform camera
{
	uniform mat4 projection;
	uniform mat4 view;
};

layout (std140) uniform light
{
	uniform mat4 light_proj;
	uniform mat4 light_view;
};

#if defined(SKINNING)
uniform mat4 palette[MAX_BONES];
#endif

uniform mat4 model;

// OUTPUT

out VS_OUT
{
    vec2 texcoord0;
    vec3 light_vector;
    vec3 camera_pos;
    vec3 frag_pos;
#if !NORMAL_MAP 
    vec3 normal;
#endif
#if RECEIVE_SHADOWS
    vec4 shadow_coord;
#endif
} vs_out;

struct VertexInfo
{
    vec3 vertex;
    vec3 normal;
#if NORMAL_MAP
    vec3 tangent;
#endif
};


void ConvertToWorld(inout VertexInfo info)
{
#if defined(SKINNING)
    mat4 skin_transform = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
                          palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];

	info.vertex  = (model*skin_transform*vec4(info.vertex, 1)).xyz;
    info.normal  = (model*skin_transform*vec4(info.normal, 0)).xyz;
#if NORMAL_MAP
    info.tangent = (model*skin_transform*vec4(info.tangent, 0)).xyz;

    // orthogonalize to avoid artifacts
    info.tangent = normalize(info.tangent-dot(info.tangent, info.normal)*info.normal);
#endif

#else 
	info.vertex  = (model*vec4(info.vertex, 1)).xyz;
    info.normal  = (model*vec4(info.normal, 0)).xyz;
#if NORMAL_MAP
    info.tangent = (model*vec4(info.tangent, 0)).xyz;

    // orthogonalize to avoid artifacts
    info.tangent = normalize(info.tangent-dot(info.tangent, info.normal)*info.normal);
#endif

#endif 
}

void ConvertOutToTangentSpace(in VertexInfo info)
{
#if NORMAL_MAP

    vec3 bitangent      = cross(info.normal, info.tangent);
    mat3 tbn            = transpose(mat3(info.tangent, bitangent, info.normal));

    vs_out.camera_pos   = tbn*vs_out.camera_pos;
    vs_out.light_vector = tbn*vs_out.light_vector;
    vs_out.frag_pos     = tbn*vs_out.frag_pos;

#endif
}

void main()
{
    VertexInfo info;

    info.vertex  = vertex;
    info.normal  = normal;
#if NORMAL_MAP
    info.tangent = tangent;
#endif

    ConvertToWorld(info);

#if RECEIVE_SHADOWS
	vs_out.shadow_coord = light_proj*light_view*vec4(info.vertex, 1.0);
#endif
    gl_Position         = projection*view*vec4(info.vertex, 1);
    vs_out.texcoord0    = texcoord0;

    vs_out.camera_pos   = transpose(mat3(view))*(-view[3].xyz);
#if LIGHT_DIRECTIONAL
    vs_out.light_vector = vec3(-light_view[0].z, -light_view[1].z, -light_view[2].z);
#else
    vs_out.light_vector = transpose(mat3(light_view))*(-light_view[3].xyz);
#endif

    vs_out.frag_pos     = info.vertex;
#if NORMAL_MAP
    ConvertOutToTangentSpace(info);
#else
    vs_out.normal       = info.normal;
#endif

}


