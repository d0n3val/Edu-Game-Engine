#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define PI 3.141597
#define CASCADE_COUNT 3
//#define SCHLICK_GGX_GSF

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
    float     normal_strength;
    float     alpha_test;
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
subroutine vec3 GetFresnel(vec3 dir0, vec3 dir1, const vec3 f0);

//////////////////// UNIFORMS ////////////////////////

layout(location=0) uniform Material material;
layout(location=20) uniform Lights lights;
layout(location=100) uniform vec3 view_pos;

#if SHADOWS_ENABLED
layout(location=110) uniform sampler2D shadow_map[3];
layout(location=113) uniform float shadow_bias;
#endif

uniform mat4 light_proj[CASCADE_COUNT];
uniform mat4 light_view[CASCADE_COUNT];

//layout(location=200) uniform samplerCube irradiance_map;
//layout(location=201) uniform samplerCube prefilter_map;
//layout(location=202) uniform sampler2D brdf_map;


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

layout(index=2) subroutine(GetFresnel) vec3 get_fresnel_schlick(vec3 dir0, vec3 dir1, const vec3 f0)
{
    float cos_theta = max(dot(dir0, dir1), 0.0); 

    return f0+(1-f0)*pow(1.0-cos_theta, 5.0);
}

layout(index=3) subroutine(GetFresnel) vec3 get_no_fresnel(vec3 dir0, vec3 dir1, const vec3 f0)
{
    return f0;
}

float sqr(float a)
{
    return a*a;
}

float BlinnNormalDistribution(float roughness, float NdotH)
{
    float smoothness = 128*(1.0-roughness); 
    return ((smoothness+2)/(2*PI))*pow(NdotH, smoothness);
}

float ImplicitGeometricShadowingFunction ()
{
    return 0.25;
}

float GGXNormalDistribution(float roughness, float NdotH)
{
    float roughnessSqr = roughness*roughness;
    float NdotHSqr = NdotH*NdotH;
    float TanNdotHSqr = (1-NdotHSqr)/NdotHSqr;
    float den = (NdotHSqr * (roughnessSqr + TanNdotHSqr));
    return (1.0/3.1415926535) * sqr(roughness/den);
}

float GGXGeometricShadowingFunction (float NdotL, float NdotV, float roughness)
{
#ifdef SCHLICK_GGX_GSF
	// Schlick-GGX GSF
    float k = roughness * 0.5;
	float SmithL = mix(NdotL, 1.0, k);
    float SmithV = mix(NdotV, 1.0, k);

    float Gs = 0.25/(SmithL*SmithV);
#else
	// GGX GSF
    float roughnessSqr = roughness*roughness;
    float NdotLSqr = NdotL*NdotL;
    float NdotVSqr = NdotV*NdotV;


	// original version
    //float SmithL = (2 * NdotL)/ (NdotL + sqrt(roughnessSqr + ( 1-roughnessSqr) * NdotLSqr));
    //float SmithV = (2 * NdotV)/ (NdotV + sqrt(roughnessSqr + ( 1-roughnessSqr) * NdotVSqr));
    // optimization using full visibility equation

    float SmithL = NdotL + sqrt(mix(NdotLSqr, 1.0, roughnessSqr));
    float SmithV = NdotV + sqrt(mix(NdotVSqr, 1.0, roughnessSqr));
	float Gs =  1.0/(SmithL * SmithV);
#endif

	return Gs;
}

vec3 directional_lighting(const vec3 normal, const vec3 view_dir, const vec3 light_dir, const vec3 light_color, 
                       const Material mat, const vec3 diffuse_color, const vec3 specular_color, float smoothness, float att)
{
    float roughness = 1.0-smoothness; 
    roughness = roughness*roughness;

	float dotNL      = max(dot(normal, light_dir), 0.000001);
	float dotNV      = max(dot(normal, view_dir), 0.000001);

    vec3 half_dir    = normalize(view_dir+light_dir);
    vec3 fresnel     = get_fresnel(light_dir, half_dir, specular_color);
    float ndf        = GGXNormalDistribution(roughness, max(0.000001, dot(half_dir, normal)));
    float gsf        = GGXGeometricShadowingFunction(dotNL, dotNV, roughness);

    return light_color*(diffuse_color*(1-fresnel)/PI+(fresnel*ndf*gsf))*dotNL*att;
}

vec3 directional_lighting(const vec3 normal, const vec3 view_dir, const DirLight light, const Material mat, 
                       const vec3 diffuse_color, const vec3 specular_color, float smoothness)
{
    return directional_lighting(normal, view_dir, -light.dir, light.color, mat, diffuse_color, specular_color, smoothness, 1.0);
}

float get_attenuation(const vec3 constants, float distance)
{
    return 1.0/(constants[0]+constants[1]*distance+constants[2]*(distance*distance));
}

vec3 point_lighting(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, const Material mat,
                 const vec3 diffuse_color, const vec3 specular_color, float smoothness)
{
    vec3 light_dir    = light.position-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float att         = get_attenuation(light.attenuation, distance);

    return directional_lighting(normal, view_dir, light_dir, light.color, mat, diffuse_color, specular_color, smoothness, att);
}

float get_cone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 spot_lighting(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, const Material mat,
                const vec3 diffuse_color, const vec3 specular_color, float smoothness)
{
    vec3 light_dir    = light.position-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float cone        = get_cone(-light_dir, light.direction, light.inner, light.outer);
    float att         = get_attenuation(light.attenuation, distance);

    return directional_lighting(normal, view_dir, light_dir, light.color, mat, diffuse_color, specular_color, smoothness, att*cone);
}

vec4 lighting(const vec3 pos, const vec3 normal, float normal_len, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    /*
    vec4 base_color      = get_diffuse_color(material, uv);
    vec4 metallic_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    float metallic       = metallic_color.r;
    vec4 diffuse_color   = vec4(base_color.rgb*(1-metallic), base_color.a);
    vec4 specular_color  = vec4(mix(vec3(0.04), base_color.rgb, metallic), metallic_color.a);

    float smoothness     = specular_color.a;
    vec3 view_dir        = normalize(view_pos-pos);

    smoothness = mat.smoothness*mat.smoothness;
    */

    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    /*
    vec4 base_color      = get_diffuse_color(material, uv);
    vec4 metallic_color  = get_specular_color(material, uv);

    float metallic       = metallic_color.r;
    vec4 diffuse_color   = vec4(base_color.rgb*(1-metallic), base_color.a);
    vec4 specular_color  = vec4(mix(vec3(0.04), base_color.rgb, metallic), metallic_color.g*metallic_color.g);
    vec3 occlusion_color = vec3(metallic_color.a);
    vec3 emissive_color  = vec3(0.0);
    */

    float smoothness     = specular_color.a;
    vec3 view_dir        = normalize(view_pos-pos);
	

    vec3 color = directional_lighting(normal, view_dir, lights.directional, mat, diffuse_color.rgb, specular_color.rgb, smoothness);

    color += point_lighting(pos, normal, view_dir, lights.points[0], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += point_lighting(pos, normal, view_dir, lights.points[1], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += point_lighting(pos, normal, view_dir, lights.points[2], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += point_lighting(pos, normal, view_dir, lights.points[3], mat, diffuse_color.rgb, specular_color.rgb, smoothness);

    color += spot_lighting(pos, normal, view_dir, lights.spots[0], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += spot_lighting(pos, normal, view_dir, lights.spots[1], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += spot_lighting(pos, normal, view_dir, lights.spots[2], mat, diffuse_color.rgb, specular_color.rgb, smoothness);
    color += spot_lighting(pos, normal, view_dir, lights.spots[3], mat, diffuse_color.rgb, specular_color.rgb, smoothness);

    color += diffuse_color.rgb*(lights.ambient.color*occlusion_color);

    /*
    const float MAX_REFLECTION_LOD = 9.0;

    smoothness = mat.smoothness;
    float roughness = 1.0-smoothness;

    vec3 F  = get_fresnel_schlick(view_dir, normal, specular_color.rgb); 
    vec3 kD = 1.0 - F;

    vec3 irradiance = texture(irradiance_map, normal).rgb;
    vec3 diffuse    = irradiance * diffuse_color.rgb / PI;

    vec3 R = reflect(-view_dir, normal);   

    vec3 prefilter_color = textureLod(prefilter_map, R,  roughness*MAX_REFLECTION_LOD).rgb;   
    vec3 env_BRDF  = texture(brdf_map, vec2(max(dot(normal, view_dir), 0.0), roughness)).rgb;
    vec3 specular = prefilter_color * (specular_color.rgb * env_BRDF.x + env_BRDF.y);
    vec3 ambient = (kD * diffuse + specular) * occlusion_color; 

    color += ambient;
    */

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
    return tbn*normal;
}

void main()
{
    vec3 normal = get_normal(fragment, material);

    float len = length(normal);
    color	  = lighting(fragment.position, normalize(normal), len, fragment.uv0, view_pos, lights, material);

    if(color.a < material.alpha_test)
    {
        discard;
    }

#if SHADOWS_ENABLED 

    for(uint i=0; i< 3; ++i)
    {
        vec4 shadow_coord = light_proj[i]*light_view[i]*vec4(fragment.position, 1.0);
        shadow_coord /= shadow_coord.w;
        shadow_coord.xy = shadow_coord.xy*0.5+0.5;

        if(shadow_coord.x >= 0.0 && shadow_coord.x < 1.0 && shadow_coord.y >= 0.0 && shadow_coord.y < 1.0 && 
           shadow_coord.z >= 0.0 && shadow_coord.z < 1.0)
        {

#if SHOW_CASCADES
            color[i] *= 20.0;
#endif


#if ENABLE_SOFT
            vec2 moments = texture(shadow_map[i], shadow_coord.xy).rg;

            if(shadow_coord.z > moments.r+shadow_bias)
            {
                float variance = moments.g - (moments.r*moments.r);

                float d = moments.x - shadow_coord.z;
                float p_max = variance / (variance + d*d);

                color.rgb = color.rgb*(p_max/2+0.5);
            }
#else

            float depth_value = texture(shadow_map[i], shadow_coord.xy).r;

            if(shadow_coord.z > depth_value+shadow_bias)
            {
                color.rgb *= 0.5;
            }
#endif
            break;
        }
    }

#endif


	// gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}
