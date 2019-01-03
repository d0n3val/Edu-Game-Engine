#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define PI 3.141597

//////////////////// STRUCTS ////////////////////////

struct Material
{
    sampler2D diffuse_map;
    vec4      diffuse_color;

    sampler2D specular_map;
    vec3      specular_color;
    float     shininess;

    sampler2D occlusion_map;

    sampler2D emissive_map;
    vec3      emissive_color;

    sampler2D normal_map;

    float     k_ambient;
    float     k_diffuse;
    float     k_specular;
};

struct AmbientLight
{
    vec3 color;
};

struct DirLight
{
    vec3 dir;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    vec3 attenuation;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 attenuation;
    float inner;
    float outer;
};

struct Lights
{
    AmbientLight ambient;
    DirLight     directional;
    PointLight   points[MAX_POINT_LIGHTS];
    uint         num_point;
    SpotLight    spots[MAX_SPOT_LIGHTS];
    uint         num_spot;
};


struct VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

subroutine vec3 GetNormal(const VertexOut vertex, const Material mat);

//////////////////// UNIFORMS ////////////////////////

layout(location=0) uniform Material material;
layout(location=20) uniform Lights lights;
layout(location=100) uniform vec3 view_pos;

layout(location=0) subroutine uniform GetNormal get_normal;


//////////////////// INPUTS ////////////////////////

in VertexOut fragment;

//////////////////// OUTPUT ////////////////////////

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

vec4 get_diffuse_color(const Material mat, const vec2 uv)
{
	vec4 diffuse = texture(mat.diffuse_map, uv)*mat.diffuse_color;
	float gamma  = 2.2;
    diffuse.rgb  = pow(diffuse.rgb, vec3(gamma));

	return diffuse;
}

vec4 get_specular_color(const Material mat, const vec2 uv)
{
    vec4 color = texture(mat.specular_map, uv);

	float gamma  = 2.2;
    color.rgb  = pow(color.rgb, vec3(gamma));
	
    return vec4(color.rgb*mat.specular_color, exp2(8*color.a*mat.shininess+1));
}

vec3 get_occlusion_color(const Material mat, const vec2 uv)
{
	float gamma = 2.2;
    return pow(texture(mat.occlusion_map, uv).rgb, vec3(gamma));
}

vec3 get_emissive_color(const Material mat, const vec2 uv)
{
    return texture(mat.emissive_map, uv).rgb*mat.emissive_color;
}

float lambert(vec3 light_dir, const vec3 normal)
{
    return max(0.0, dot(normal, -light_dir));
}

float specular_blinn(vec3 light_dir, const vec3 pos, const vec3 normal, const vec3 view_pos, const float shininess)
{
    vec3 view_dir    = normalize(view_pos-pos);
    vec3 half_dir    = normalize(view_dir-light_dir);
    float sp         = max(dot(normal, half_dir), 0.0);

    return pow(sp, shininess); 
}

vec3 directional_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const DirLight light, const Material mat, 
                       const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    float lambert		= lambert(lights.directional.dir, normal);	
    float specular		= specular_blinn(lights.directional.dir, pos, normal, view_pos, shininess);
	
	vec3 view_dir     = normalize(view_pos-pos);
    float cos_theta   = max(dot(view_dir, normal), 0.0);
	float cos_theta2  = max(dot(-lights.directional.dir, normal), 0.0);
    vec3 r0           = specular_color;

    float norm_factor = (shininess+4.0)/(8.0);
    vec3 fresnel      = vec3(r0+(1-r0)*pow(1.0-cos_theta, 5.0));
    vec3 new_specular = fresnel*norm_factor;
    vec3 new_diffuse  = (1-fresnel)*diffuse_color;

    return lights.directional.color*((new_diffuse*lambert+new_specular*specular));
}

float get_attenuation(const vec3 constants, float distance)
{
    return 1.0/(constants[0]+constants[1]*distance+constants[2]*(distance*distance));
}

vec3 point_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const PointLight light, const Material mat,
                 const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    vec3 light_dir = pos-light.position;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float att      = get_attenuation(light.attenuation, distance);

    float lambert  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, pos, normal, view_pos, shininess);

    float norm_factor = (shininess+4.0)/(8.0);
    vec3 new_specular = (1.0-diffuse_color)*specular_color*norm_factor;
    vec3 new_diffuse  = diffuse_color;

    return light.color*(att*(new_diffuse*lambert+new_specular*specular));
}

float get_cone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 spot_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const SpotLight light, const Material mat,
                const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    vec3 light_dir = pos-light.position;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float cone     = get_cone(light_dir, light.direction, light.inner, light.outer);
    float att      = get_attenuation(light.attenuation, distance);

    float lambert  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, pos, normal, view_pos, shininess);

    float norm_factor = (shininess+4.0)/(8.0);
    vec3 new_specular = (1.0-diffuse_color)*specular_color*norm_factor;
    vec3 new_diffuse  = diffuse_color;

    return light.color*((att*cone)*(new_diffuse*lambert+new_specular*specular));
}

vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    vec3 color = directional_blinn(pos, normal, view_pos, lights.directional, mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);

    color += point_blinn(pos, normal, view_pos, lights.points[0], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += point_blinn(pos, normal, view_pos, lights.points[1], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += point_blinn(pos, normal, view_pos, lights.points[2], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += point_blinn(pos, normal, view_pos, lights.points[3], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);

    color += spot_blinn(pos, normal, view_pos, lights.spots[0], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += spot_blinn(pos, normal, view_pos, lights.spots[1], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += spot_blinn(pos, normal, view_pos, lights.spots[2], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);
    color += spot_blinn(pos, normal, view_pos, lights.spots[3], mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);

    color += diffuse_color.rgb*(lights.ambient.color*occlusion_color*material.k_ambient);
    color += emissive_color;

    return vec4(color, diffuse_color.a); 
}

mat3 create_tbn(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal); // skinning forces this
    vec3 bitangent     = cross(normal, ortho_tangent);

    return mat3(tangent, bitangent, normal);
}

layout(index=0) subroutine(GetNormal) vec3 get_normal_from_interpolator(const VertexOut vertex, const Material mat)
{
    return normalize(vertex.normal);
}

layout(index=1) subroutine(GetNormal) vec3 get_normal_from_texture(const VertexOut vertex, const Material mat)
{
    mat3 tbn = create_tbn(normalize(vertex.normal), normalize(vertex.tangent));
    return tbn*normalize(texture(mat.normal_map, vertex.uv0).xyz*2.0-1.0);
}

void main()
{
    color	  = blinn(fragment.position, get_normal(fragment, material), fragment.uv0, view_pos, lights, material);
	color.rgb = pow(color.rgb, vec3(1.0/2.2));
    //color = texture(material.normal_map, fragment.uv0); //vec4(get_normal(fragment, material), 1.0);
    //color = vec4(vec3(texture(material.specular_map, fragment.uv0).a), 1.0); //vec4(get_normal(fragment, material), 1.0);
}
