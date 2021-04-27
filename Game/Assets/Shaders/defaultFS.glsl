#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define PI 3.141597
#define CASCADE_COUNT 3

#define DIFFUSE_MAP_INDEX 0
#define SPECULAR_MAP_INDEX 1
#define NORMAL_MAP_INDEX 2
#define OCCLUSION_MAP_INDEX 3
#define EMISSIVE_MAP_INDEX 4
#define LIGHT_MAP_INDEX 5
#define MAP_COUNT 6

#define DIFFUSE_MAP_FLAG   0x00000001u
#define SPECULAR_MAP_FLAG  0x00000002u
#define NORMAL_MAP_FLAG    0x00000004u
#define OCCLUSION_MAP_FLAG 0x00000008u
#define EMISSIVE_MAP_FLAG  0x00000010u
#define LIGHT_MAP_FLAG     0x00000020u

//////////////////// STRUCTS ////////////////////////

layout(std140, row_major) uniform Camera 
{
    mat4 proj;
    mat4 view;
    vec3 view_pos;
} camera;

layout(std140) uniform Material
{
    vec4      diffuseColor;
    vec4      specularColor;
    vec4      emissiveColor;
    float     smoothness;
    float     normalStrength;
    float     alphaTest;
    uint      mapMask;
} material;

uniform sampler2D materialMaps[MAP_COUNT];

uniform samplerCube diffuseIBL;

struct AmbientLight
{
    vec4 color;
};

struct DirLight
{
    vec4 dir;
    vec4 color;
};

struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 attenuation;
};

struct SpotLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 attenuation;
    float inner;
    float outer;
};

layout(std140) uniform Lights
{
    AmbientLight ambient;
    DirLight     directional;
    PointLight   points[MAX_POINT_LIGHTS];
    uint         num_point;
    SpotLight    spots[MAX_SPOT_LIGHTS];
    uint         num_spot;
} lights;

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

//////////////////// UNIFORMS ////////////////////////

layout(location=100) uniform vec3 view_pos;

#if SHADOWS_ENABLED

#if ENABLE_SOFT
layout(location=110) uniform sampler2D shadow_map[3];
#else
layout(location=110) uniform sampler2DShadow shadow_map[3];
#endif

layout(location=113) uniform float shadow_bias;
layout(location=114) uniform int kernel_half_size;
#endif

uniform vec2 map_size[CASCADE_COUNT];


//////////////////// INPUTS ////////////////////////

in VertexOut fragment;
in vec4 shadow_coord[3];

//////////////////// OUTPUT ////////////////////////

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

float Sq(float a)
{
    return a*a;
}

void GetMaterial(out vec4 diffuse, out vec3 specular, out float smoothness, out vec3 occlusion, out vec3 emissive)
{
	diffuse    = material.diffuseColor;
    specular   = material.specularColor.rgb;
    smoothness = material.smoothness;
    occlusion  = vec3(1.0);
    emissive   = material.emissiveColor.rgb;

    if((material.mapMask & DIFFUSE_MAP_FLAG) != 0)
    {
        diffuse *= texture(materialMaps[DIFFUSE_MAP_INDEX], fragment.uv0);
    }

    if((material.mapMask & SPECULAR_MAP_FLAG) != 0)
    {
        vec4 tmp = texture(materialMaps[SPECULAR_MAP_INDEX], fragment.uv0);
        specular   *= tmp.rgb;
        smoothness *= tmp.a;
    }

    if((material.mapMask & OCCLUSION_MAP_FLAG) != 0)
    {
        occlusion *= texture(materialMaps[OCCLUSION_MAP_INDEX], fragment.uv0).rgb;
    }

    if((material.mapMask & EMISSIVE_MAP_FLAG) != 0)
    {
        emissive *= texture(materialMaps[EMISSIVE_MAP_INDEX], fragment.uv0).rgb;
    }
}

vec3 GetFresnel(vec3 dir0, vec3 dir1, const vec3 f0)
{
    float cos_theta = max(dot(dir0, dir1), 0.0); 

    return f0+(1-f0)*pow(1.0-cos_theta, 5.0);
}

float GGXNDF(float roughness, float NdotH)
{
    float sq_rough = Sq(roughness);
    float sq_ndoth = Sq(NdotH);
    
    return sq_rough/(PI*Sq(sq_ndoth*(sq_rough-1.0)+1.0));
}

float SMITHVSF(float NdotL, float NdotV, float roughness)
{
    float GGXV = NdotL * (NdotV * (1.0 - roughness) + roughness);
    float GGXL = NdotV * (NdotL * (1.0 - roughness) + roughness);
    return 0.5 / max(0.00001, (GGXV + GGXL));
}

vec3 GGXShading(const vec3 normal, const vec3 view_dir, const vec3 light_dir, const vec3 light_color, 
                const vec3 diffuseColor, const vec3 specularColor, float smoothness, float att)
{
    float roughness  = Sq(1.0-smoothness); 

    float dotNL      = max(dot(normal, light_dir), 0.000001);
    float dotNV      = max(dot(normal, view_dir), 0.000001);

    vec3 half_dir    = normalize(view_dir+light_dir);
    vec3 fresnel     = GetFresnel(light_dir, half_dir, specularColor);
    float ndf        = GGXNDF(roughness, max(0.000001, dot(half_dir, normal)));
    float vsf        = SMITHVSF(dotNL, dotNV, roughness);

    return (diffuseColor*(1-specularColor)+(fresnel*ndf*vsf))*light_color*dotNL*att;
}

float GetAttenuation(const vec3 constants, float distance)
{
    return 1.0/(constants[0]+constants[1]*distance+constants[2]*(distance*distance));
}

float GetCone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 Directional(const vec3 normal, const vec3 view_dir, const DirLight light, const vec3 diffuseColor, const vec3 specularColor, float smoothness)
{
    return GGXShading(normal, view_dir, -light.dir.xyz, light.color.rgb, diffuseColor, specularColor, smoothness, 1.0);
}

vec3 Point(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, 
           const vec3 diffuseColor, const vec3 specularColor, float smoothness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float att         = GetAttenuation(light.attenuation.xyz, distance);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb, diffuseColor, specularColor, smoothness, att);
}

vec3 Spot(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, 
                   const vec3 diffuseColor, const vec3 specularColor, float smoothness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float cone        = GetCone(-light_dir, light.direction.xyz, light.inner, light.outer);
    float att         = GetAttenuation(light.attenuation.xyz, distance);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb, diffuseColor, specularColor, smoothness, att*cone);
}

vec4 Shading(const in vec3 pos, const in vec3 normal)
{
    vec4 diffuseColor;
    vec3 specularColor;
    float smoothness;
    vec3 occlusion_color;
    vec3 emissiveColor;

    GetMaterial(diffuseColor, specularColor, smoothness, occlusion_color, emissiveColor);

    vec3 view_dir = normalize(camera.view_pos-pos);
	
    vec3 color = Directional(normal, view_dir, lights.directional, diffuseColor.rgb, specularColor.rgb, smoothness);

    for(uint i=0; i< lights.num_point; ++i)
    {
        color += Point(pos, normal, view_dir, lights.points[i], diffuseColor.rgb, specularColor, smoothness);
    }

    for(uint i=0; i< lights.num_spot; ++i)
    {
        color += Spot(pos, normal, view_dir, lights.spots[i], diffuseColor.rgb, specularColor, smoothness);
    }

    //color += diffuseColor.rgb*(lights.ambient.color.rgb*occlusion_color);

    vec3 irradiance = texture(diffuseIBL, normal).rgb;


    color += irradiance*(diffuseColor.rgb*(1.0-specularColor))*occlusion_color;;

    color += emissiveColor;

    return vec4(color, diffuseColor.a); 
}

mat3 CreateTBN(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal); // skinning forces this
    vec3 bitangent     = cross(normal, ortho_tangent);

    return mat3(tangent, bitangent, normal);
}

vec3 GetNormal()
{
    if((material.mapMask & NORMAL_MAP_FLAG) != 0)
    {
        vec3 normal = texture(materialMaps[NORMAL_MAP_INDEX], fragment.uv0).xyz*2.0-1.0;
        normal.xy *= material.normalStrength;
        normal = normalize(normal);

        mat3 tbn = CreateTBN(normalize(fragment.normal), normalize(fragment.tangent));
        return normalize(tbn*normal);
    }

    return normalize(fragment.normal);
}

void main()
{

    vec4 baked = texture(materialMaps[LIGHT_MAP_INDEX], fragment.uv1);
    color      = Shading(fragment.position, GetNormal());
    color.rgb += baked.rgb;

    if(color.a < material.alphaTest)
    {
        discard;
    }

#if SHADOWS_ENABLED 

    for(uint i=0; i< 3; ++i)
    {
        if(shadow_coord[i].x >= 0.0 && shadow_coord[i].x < 1.0 && 
           shadow_coord[i].y >= 0.0 && shadow_coord[i].y < 1.0 && 
           shadow_coord[i].z >= 0.0 && shadow_coord[i].z < 1.0)
        {

#if SHOW_CASCADES
            color[i] *= 20.0;
#endif


#if ENABLE_SOFT

            vec2 moments = texture(shadow_map[i], shadow_coord[i].xy).rg;

            if(shadow_coord[i].z > moments.r+shadow_bias)
            {
                float variance = moments.g - (moments.r*moments.r);

                float d = moments.x - shadow_coord[i].z;
                float p_max = variance / (variance + d*d);

                color.rgb = color.rgb*(p_max/2+0.5);
            }


#else 

            // Compute PCF

            vec3 coord = vec3(shadow_coord[i].xy, shadow_coord[i].z-shadow_bias);

            float xOffset = 1.0/map_size[i].x;
            float yOffset = 1.0/map_size[i].y;

            float shadow_factor = 0.0;
            int total_samples = (kernel_half_size*2+1)*(kernel_half_size*2+1);

            for (int y = -kernel_half_size ; y <= kernel_half_size ; y++) 
            {
                for (int x = -kernel_half_size ; x <= kernel_half_size ; x++) 
                {
                    vec2 Offsets = vec2(x * xOffset, y * yOffset);
                    coord.xy = shadow_coord[i].xy + Offsets;
                    shadow_factor += texture(shadow_map[i], coord);
                }
            }

            shadow_factor /= float(total_samples);

            color.rgb = mix(vec3(0.0), color.rgb, 0.5+shadow_factor*0.5);
#endif


            break;
        }
    }

#endif


	// gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}
