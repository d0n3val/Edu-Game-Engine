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
    float     smoothness;

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
subroutine float GetFresnel(vec3 dir0, vec3 dir1);

//////////////////// UNIFORMS ////////////////////////

layout(location=0) uniform Material material;
layout(location=20) uniform Lights lights;
layout(location=100) uniform vec3 view_pos;

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

    //return vec4(color.rgb*mat.specular_color, exp2(9*color.a*mat.smoothness+1));
    return vec4(color.rgb*mat.specular_color, color.a*mat.smoothness);
}

vec3 get_occlusion_color(const Material mat, const vec2 uv)
{
    return texture(mat.occlusion_map, uv).rgb;
}

vec3 get_emissive_color(const Material mat, const vec2 uv)
{
    return texture(mat.emissive_map, uv).rgb*mat.emissive_color;
}

layout(index=2) subroutine(GetFresnel) float get_fresnel_schlick(vec3 dir0, vec3 dir1)
{
    float cos_theta = max(dot(dir0, dir1), 0.0); 

    return pow(1.0-cos_theta, 5.0);
}

layout(index=3) subroutine(GetFresnel) float get_no_fresnel(vec3 dir0, vec3 dir1)
{
    return 0.0;
}

float lambert(vec3 light_dir, const vec3 normal)
{
    return max(0.0, dot(normal, -light_dir));
}

float specular_blinn(const vec3 light_dir, const vec3 normal, const vec3 view_dir, const float smoothness, out float fresnel)
{
    vec3 half_dir    = normalize(view_dir-light_dir);
    float sp         = max(dot(normal, half_dir), 0.0);
    fresnel          = get_fresnel(view_dir, half_dir);

    return pow(sp, smoothness);
}

float GGXGSF(float ndotl, float ndotv, float roughness)
{
    float roughsqr = roughness*roughness;
    float ndotlsqr = ndotl*ndotl;
    float ndotvsqr = ndotv*ndotv;

    float smithL = (2*ndotl)/(ndotl+sqrt(roughsqr+(1-roughsqr)*ndotlsqr));
    float smithV = (2*ndotv)/(ndotv+sqrt(roughsqr+(1-roughsqr)*ndotvsqr));

    return smithL*smithV;
}

float GGXNDF(float roughness, float ndoth)
{
    float roughsqr = roughness*roughness;
    float ndothsqr = ndoth*ndoth;
    float tanndothsqr = (1-ndothsqr)/ndothsqr;
    return (1.0/PI)*sqrt(roughness/(ndothsqr*(roughsqr*tanndothsqr)));
}

vec3 directional_blinn(const vec3 normal, const vec3 view_dir, const DirLight light, const Material mat, 
                       const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float smoothness)
{

    float roughness    = 1.0-pow(smoothness, 4);
	smoothness	       = exp2(9*smoothness+1);
    float fresnel      = 0.0;
    float ndotl        = max(0.0, dot(normal, -lights.directional.dir));
    float ndotv        = max(0.0, dot(normal, view_dir));	
    float ndoth        = max(0.00001, dot(normal, normalize(view_dir-lights.directional.dir)));	
    float specular     = specular_blinn(lights.directional.dir, normal, view_dir, smoothness, fresnel);
    vec3 fresnel_color = specular_color+(1.0-specular_color)*fresnel;
    float den          = max(0.0001, ndotl*ndotv);
    //roughness = roughness*roughness;
    float visibility   = 1.0; //GGXGSF(ndotl, ndotv, roughness)/(4.0*den); 
    float ndf          = GGXNDF(roughness, ndoth);
    //float ndf          = specular*norm_factor;
	
    return lights.directional.color*(diffuse_color*(1-specular_color)+fresnel_color*(visibility*ndf))*ndotl;
}

float get_attenuation(const vec3 constants, float distance)
{
    return 1.0/(constants[0]+constants[1]*distance+constants[2]*(distance*distance));
}

vec3 point_blinn(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, const Material mat,
                 const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float smoothness)
{
    float roughness    = 1.0-pow(smoothness, 4);
	smoothness	   = exp2(9*smoothness+1);
    vec3 light_dir = pos-light.position;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float att      = get_attenuation(light.attenuation, distance);

    float fresnel  = 0.0;
    float ndotl        = max(0.0, dot(normal, -lights.directional.dir));
    float ndotv        = max(0.0, dot(normal, view_dir));	
    float ndoth        = max(0.00001, dot(normal, normalize(view_dir-lights.directional.dir)));	
    float specular     = specular_blinn(light_dir, normal, view_dir, smoothness, fresnel);
    vec3 fresnel_color = specular_color+(1-specular_color)*fresnel;
    float den          = max(0.0001, ndotl*ndotv);
    //float roughness    = 1.0-min(smoothness, 0.9);
    //roughness = roughness*roughness;
    float visibility   = 1.0; //GGXGSF(ndotl, ndotv, roughness)/(4.0*den); 
    float ndf          = GGXNDF(roughness, ndoth);
    //float ndf          = specular*norm_factor;

    return light.color*(diffuse_color*(1-specular_color)+fresnel_color*(visibility*ndf))*ndotl*att;
}

float get_cone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 spot_blinn(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, const Material mat,
                const vec3 diffuse_color, const vec3 specular_color, float norm_factor, float smoothness)
{
    vec3 light_dir = pos-light.position;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float cone     = get_cone(light_dir, light.direction, light.inner, light.outer);
    float att      = get_attenuation(light.attenuation, distance);

    float fresnel  = 0.0;
    float lambert  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, normal, view_dir, smoothness, fresnel);
    vec3 fresnel_color = specular_color+(1-specular_color)*fresnel;

    return light.color*(diffuse_color*(1-specular_color)+fresnel_color*(specular*norm_factor))*(lambert*att*cone);
}

vec4 blinn(const vec3 pos, const vec3 normal, float normal_len, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    float smoothness      = specular_color.a;
    // better smoothness 
    //smoothness           = normal_len*smoothness/(normal_len+smoothness*(1-normal_len));
	//smoothness	         = exp2(9*smoothness+1);
    vec3 view_dir        = normalize(view_pos-pos);
	float norm_factor    = (exp2(9*smoothness+1)+4.0)/(8.0);
	
    vec3 color = directional_blinn(normal, view_dir, lights.directional, mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);

    color += point_blinn(pos, normal, view_dir, lights.points[0], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += point_blinn(pos, normal, view_dir, lights.points[1], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += point_blinn(pos, normal, view_dir, lights.points[2], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += point_blinn(pos, normal, view_dir, lights.points[3], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);

    //color += spot_blinn(pos, normal, view_dir, lights.spots[0], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += spot_blinn(pos, normal, view_dir, lights.spots[1], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += spot_blinn(pos, normal, view_dir, lights.spots[2], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);
    //color += spot_blinn(pos, normal, view_dir, lights.spots[3], mat, diffuse_color.rgb, specular_color.rgb, norm_factor, smoothness);

    //color += diffuse_color.rgb*(lights.ambient.color*occlusion_color*material.k_ambient);
    //color += emissive_color;

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
    return tbn*(texture(mat.normal_map, vertex.uv0).xyz*2.0-1.0);
}

void main()
{
    vec3 normal = get_normal(fragment, material);
    float len   = length(normal);
    color	    = blinn(fragment.position, normalize(normal), len, fragment.uv0, view_pos, lights, material);

	// gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}
