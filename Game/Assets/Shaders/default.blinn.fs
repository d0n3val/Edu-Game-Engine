// todo: Shadows, shininess instead of smoothness and factorize directional light
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
    float     normal_strength;
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
subroutine vec3 GetFresnel(vec3 L, vec3 N, vec3 F0);

//////////////////// UNIFORMS ////////////////////////

layout(location=0) uniform Material material;
layout(location=20) uniform Lights lights;
layout(location=100) uniform vec3 view_pos;

// todo: shadows

layout(location=0) subroutine uniform GetNormal get_normal;
layout(location=1) subroutine uniform GetFresnel get_fresnel;


//////////////////// INPUTS ////////////////////////

in VertexOut fragment;

//////////////////// OUTPUT ////////////////////////

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

vec4 get_diffuse_color(const Material mat, const vec2 uv)
{
	vec4 diffuse = texture(mat.diffuse_map, uv)*mat.diffuse_color;

	return diffuse;
}

vec4 get_specular_color(const Material mat, const vec2 uv)
{
    vec4 color = texture(mat.specular_map, uv);

    return vec4(color.rgb*mat.specular_color, exp2(7*color.a*mat.shininess+1));
}

vec3 get_occlusion_color(const Material mat, const vec2 uv)
{
    return texture(mat.occlusion_map, uv).rgb;
}

vec3 get_emissive_color(const Material mat, const vec2 uv)
{
    return texture(mat.emissive_map, uv).rgb*mat.emissive_color;
}

float pow5(float a)
{
    return a*a*a*a*a;
}

layout(index=2) subroutine(GetFresnel) vec3 get_fresnel_schlick(vec3 L, vec3 N, vec3 F0)
{
    float cos_theta = max(dot(L, N), 0.0); 

    return F0+(1-F0)*pow5(1.0-cos_theta);
}

layout(index=3) subroutine(GetFresnel) vec3 get_no_fresnel(vec3 L, vec3 N, vec3 F0)
{
    return F0;
}

float lambert(vec3 light_dir, const vec3 normal)
{
    return max(0.0, dot(normal, light_dir));
}

float specular_blinn(vec3 light_dir, const vec3 view_dir, const vec3 normal, const float shininess)
{
    vec3 half_dir    = normalize(view_dir+light_dir);
    float sp         = max(dot(normal, half_dir), 0.0);

    return pow(sp, shininess); 
}

vec3 directional_blinn(const vec3 pos, const vec3 normal, const vec3 view_dir, const DirLight light, const Material mat, 
                       const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float shininess)
{
    float lambert  = lambert(-light.dir, normal);	
    float specular = specular_blinn(-light.dir, view_dir, normal, shininess);
    vec3 fresnel   = get_fresnel(-light.dir, normal, specular_color);
	
    return light.color*(diffuse_color*(1-fresnel)/PI+fresnel*specular*norm_factor)*lambert;
}

float get_attenuation(const vec3 constants, float distance)
{
    return 1.0/(constants[0]+constants[1]*distance+constants[2]*(distance*distance));
}

vec3 point_blinn(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, const Material mat,
                 const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float shininess)
{
    vec3 light_dir = light.position-pos;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float att      = get_attenuation(light.attenuation, distance);

    float lambert  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, view_dir, normal, shininess);
    vec3 fresnel   = get_fresnel(light_dir, normal, specular_color);

    return light.color*(diffuse_color*(1-fresnel)+fresnel*(specular*norm_factor))*(lambert*att);
}

float get_cone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 spot_blinn(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, const Material mat,
                const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float shininess)
{
    vec3 light_dir = light.position-pos;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float att      = get_attenuation(light.attenuation, distance);
    float cone     = get_cone(light_dir, light.direction, light.inner, light.outer);

    float lambert  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, view_dir, normal, shininess);
    vec3 fresnel   = get_fresnel(light_dir, normal, specular_color);

    return light.color*(diffuse_color*(1-fresnel)+fresnel*(specular*norm_factor))*(lambert*att*cone);
}

vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    // make energy conserving blinn brdf

    float shininess	= specular_color.a;
    float norm_factor   = (shininess+8.0)/(8.0*PI);
    vec3 view_dir       = normalize(view_pos-pos);
	
    vec3 color = directional_blinn(pos, normal, view_dir, lights.directional, mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);

    color += point_blinn(pos, normal, view_dir, lights.points[0], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += point_blinn(pos, normal, view_dir, lights.points[1], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += point_blinn(pos, normal, view_dir, lights.points[2], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += point_blinn(pos, normal, view_dir, lights.points[3], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);

    color += spot_blinn(pos, normal, view_dir, lights.spots[0], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += spot_blinn(pos, normal, view_dir, lights.spots[1], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += spot_blinn(pos, normal, view_dir, lights.spots[2], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);
    color += spot_blinn(pos, normal, view_dir, lights.spots[3], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, shininess);

    color += diffuse_color.rgb*(lights.ambient.color*occlusion_color);
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
    vec3 normal = texture(mat.normal_map, vertex.uv0).xyz*2.0-1.0;
    normal.xy *= mat.normal_strength;
    normal = normalize(normal);

    mat3 tbn = create_tbn(normalize(vertex.normal), normalize(vertex.tangent));
    return tbn*normalize(normal*2.0-1.0);
}

void main()
{
    vec3 normal = get_normal(fragment, material);
    color	    = blinn(fragment.position, normal, fragment.uv0, view_pos, lights, material);

    // gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}
