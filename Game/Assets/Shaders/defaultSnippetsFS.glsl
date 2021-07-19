--- PREFIX

#version 440

--- DATA

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
#define DETAIL_MASK_MAP_INDEX 6
#define SECOND_DIFFUSE_MAP_INDEX 7
#define SECOND_SPECULAR_MAP_INDEX 8
#define SECOND_NORMAL_MAP_INDEX 9
#define MAP_COUNT 10

#define DIFFUSE_MAP_FLAG        0x00000001u
#define SPECULAR_MAP_FLAG       0x00000002u
#define NORMAL_MAP_FLAG         0x00000004u
#define OCCLUSION_MAP_FLAG      0x00000008u
#define EMISSIVE_MAP_FLAG       0x00000010u
#define LIGHT_MAP_FLAG          0x00000020u
#define DETAIL_MASK_FLAG        0x00000040u
#define SECOND_DIFFUSE_FLAG     0x00000080u
#define SECOND_SPECULAR_FLAG    0x00000100u
#define SECOND_NORMAL_FLAG      0x00000200u

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
    vec2      uv_tiling;
    vec2      uv_offset;
    vec2      uv_secondary_tiling;
    vec2      uv_secondary_offset;
    float     smoothness;
    float     normalStrength;
    float     alphaTest;
    uint      mapMask;
} material;

uniform sampler2D materialMaps[MAP_COUNT];

uniform sampler2D   ambientOcclusion;
uniform samplerCube diffuseIBL;
uniform samplerCube prefilteredIBL;
uniform sampler2D   environmentBRDF;
uniform int         prefilteredLevels;

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

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

float Sq(float a)
{
    return a*a;
}

mat3 CreateTBN(const vec3 normal, const vec3 tangent);
void GetMaterial(out vec4 diffuse, out vec3 specular, out float smoothness, out vec3 occlusion, out vec3 emissive, out vec3 normal);
vec4 Shading(const in vec3 pos, const in vec3 normal, vec4 diffuse, vec3 specular, float smoothness, vec3 occlusion, vec3 emissive);
vec3 ComputeShadow(in vec3 color);
vec3 applyFog( in vec3  rgb, in float dist, in vec3 rayOrig, in vec3 rayDir);

--- MAIN

void main()
{
    vec4 baked = texture(materialMaps[LIGHT_MAP_INDEX], fragment.uv1);

    vec4 diffuse;
    vec3 specular;
    float smoothness;
    vec3 occlusion;
    vec3 emissive;
    vec3 normal;

    GetMaterial(diffuse, specular, smoothness, occlusion, emissive, normal);

    color      = Shading(fragment.position, normal, diffuse, specular, smoothness, occlusion, emissive);
    color.rgb += baked.rgb;

    if(color.a < material.alphaTest)
    {
        discard;
    }

    color.rgb = ComputeShadow(color.rgb);

    //color.rgb = applyFog(color.rgb, distance(fragment.position, camera.view_pos), camera.view_pos, normalize(fragment.position-camera.view_pos));

	// gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}

vec3 applyFog( in vec3  rgb, in float dist, in vec3 rayOrig, in vec3 rayDir) 
{
    float b = 0.2;
    float a = 0.02;
    float fogAmount = (a/b) * exp(-rayOrig.y*b) * (1.0-exp( -dist*rayDir.y*b ))/rayDir.y;
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}

void GetMaterial(out vec4 diffuse, out vec3 specular, out float smoothness, out vec3 occlusion, out vec3 emissive, out vec3 normal)
{
    diffuse    = material.diffuseColor;
    specular   = material.specularColor.rgb;
    smoothness = material.smoothness;
    occlusion  = vec3(1.0);
    emissive   = material.emissiveColor.rgb;

    vec2 uv0   = fragment.uv0*material.uv_tiling+material.uv_offset;
    vec2 uv1   = fragment.uv0*material.uv_secondary_tiling+material.uv_secondary_offset;

    if((material.mapMask & DIFFUSE_MAP_FLAG) != 0)
    {
        diffuse *= texture(materialMaps[DIFFUSE_MAP_INDEX], uv0);
    }

    if((material.mapMask & SPECULAR_MAP_FLAG) != 0)
    {
        vec4 tmp = texture(materialMaps[SPECULAR_MAP_INDEX], uv0);
        specular   *= tmp.rgb;
        smoothness *= tmp.a;
    }

    if((material.mapMask & OCCLUSION_MAP_FLAG) != 0)
    {
        occlusion *= texture(materialMaps[OCCLUSION_MAP_INDEX], uv0).rgb;
    }

    if((material.mapMask & EMISSIVE_MAP_FLAG) != 0)
    {
        emissive *= texture(materialMaps[EMISSIVE_MAP_INDEX], uv0).rgb;
    }

    vec3 tex_normal = vec3(0.0);
    bool has_tex_normal = false;

    if((material.mapMask & NORMAL_MAP_FLAG) != 0)
    {
        tex_normal = texture(materialMaps[NORMAL_MAP_INDEX], uv0).xyz*2.0-1.0;
        tex_normal.xy *= material.normalStrength;
        tex_normal = normalize(tex_normal);

        has_tex_normal = true;
    }

    if((material.mapMask & DETAIL_MASK_FLAG) != 0)
    {
        float blend = texture(materialMaps[DETAIL_MASK_MAP_INDEX], fragment.uv0).a;

        if((material.mapMask & SECOND_DIFFUSE_FLAG) != 0)
        {
            diffuse = mix(diffuse, texture(materialMaps[SECOND_DIFFUSE_MAP_INDEX], uv1), blend);
        }

        if((material.mapMask & SECOND_SPECULAR_FLAG) != 0)
        {
            vec4 spec  = texture(materialMaps[SECOND_SPECULAR_MAP_INDEX], uv1);
            specular   = mix(specular, spec.rgb, blend);
            smoothness = mix(smoothness, spec.a, blend);
        }

        if((material.mapMask & SECOND_NORMAL_FLAG) != 0)
        {
            vec3 second_tex_normal = texture(materialMaps[SECOND_NORMAL_MAP_INDEX], uv1).xyz*2.0-1.0;
            // \todo: second_tex_normal.xy *= material.normalStrength;
            second_tex_normal = normalize(second_tex_normal);

            if(has_tex_normal)
            {
                tex_normal = normalize(mix(tex_normal, second_tex_normal, blend));
            }
            else
            {
                tex_normal = second_tex_normal;
            }

            has_tex_normal = true;
        }
    }

    if(has_tex_normal)
    {
        mat3 tbn = CreateTBN(normalize(fragment.normal), normalize(fragment.tangent));
        normal = normalize(tbn*tex_normal);
    }
    else
    {
        normal = normalize(fragment.normal);
    }
}

vec3 GetFresnel(vec3 dir0, vec3 dir1, const vec3 f0)
{
    float cos_theta = max(dot(dir0, dir1), 0.0); 

    return f0+(1-f0)*pow(1.0-cos_theta, 5.0);
}

// from filament
float GGXNDF(float roughness, float NdotH)
{
    float a = NdotH*roughness;
    float k = roughness/(1.0-NdotH*NdotH+a*a);
    return k*k*(1.0/PI);
}

// from filament (note: no opt)
float SMITHVSF(float NdotL, float NdotV, float roughness)
{
    float a2 = roughness * roughness;
    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

vec3 GGXShading(const vec3 normal, const vec3 view_dir, const vec3 light_dir, const vec3 light_color, 
                const vec3 diffuseColor, const vec3 specularColor, float roughness, float att)
{
    float dotNL      = max(dot(normal, light_dir), 0.001);
    float dotNV      = max(dot(normal, view_dir), 0.001);

    vec3 half_dir    = normalize(view_dir+light_dir);
    vec3 fresnel     = GetFresnel(light_dir, half_dir, specularColor);
    float ndf        = GGXNDF(roughness, max(0.001, dot(half_dir, normal)));
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

vec3 Directional(const vec3 normal, const vec3 view_dir, const DirLight light, const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    return GGXShading(normal, view_dir, -light.dir.xyz, light.color.rgb, diffuseColor, specularColor, roughness, 1.0);
}

vec3 Point(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, 
           const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float att         = GetAttenuation(light.attenuation.xyz, distance);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb, diffuseColor, specularColor, roughness, att);
}

vec3 Spot(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, 
                   const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;

    float cone        = GetCone(-light_dir, light.direction.xyz, light.inner, light.outer);
    float att         = GetAttenuation(light.attenuation.xyz, distance);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb, diffuseColor, specularColor, roughness, att*cone);
}

vec4 Shading(const in vec3 pos, const in vec3 normal, vec4 diffuse, vec3 specular, float smoothness, vec3 occlusion, vec3 emissive)
{
    vec3 V           = normalize(camera.view_pos-pos);
    vec3 R           = reflect(-V, normal);
    float NdotV      = max(dot(normal, V), 0.00001);
    float roughness  = Sq(1.0-smoothness); 
    
    vec3 color = Directional(normal, V, lights.directional, diffuse.rgb, specular.rgb, roughness);

    for(uint i=0; i< lights.num_point; ++i)
    {
        color += Point(pos, normal, V, lights.points[i], diffuse.rgb, specular, roughness);
    }

    for(uint i=0; i< lights.num_spot; ++i)
    {
        color += Spot(pos, normal, V, lights.spots[i], diffuse.rgb, specular, roughness);
    }

    // Add Indirect lighting 
    vec3 irradiance = texture(diffuseIBL, normal).rgb;
    vec3 radiance   = textureLod(prefilteredIBL, R, roughness*(prefilteredLevels-1)).rgb;
    vec2 fab        = texture(environmentBRDF, vec2(NdotV, roughness)).rg;
    vec3 indirect   = (diffuse.rgb*(1-specular.rgb))*irradiance+radiance*(specular.rgb*fab.x+fab.y);


    // Compute ambient occlusion
    vec4 projectedPos = camera.proj*camera.view*vec4(fragment.position, 1.0);
    vec2 occlusionUV  = (projectedPos.xy/projectedPos.w)*0.5+0.5;

    vec3 occlusionFactor = vec3(texture(ambientOcclusion, occlusionUV).r);

    color += indirect*occlusionFactor;

    color += emissive;

    return vec4(color, diffuse.a); 
}

mat3 CreateTBN(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal); // skinning forces this
    vec3 bitangent     = cross(normal, ortho_tangent);

    return mat3(tangent, bitangent, normal);
}

--- SHADOW

#if ENABLE_SOFT
layout(location=110) uniform sampler2D shadow_map[3];
#else
layout(location=110) uniform sampler2DShadow shadow_map[3];
#endif

layout(location=113) uniform float shadow_bias;
layout(location=114) uniform int kernel_half_size;

uniform vec2 map_size[CASCADE_COUNT];
in vec4 shadow_coord[3];


vec3 ComputeShadow(in vec3 color)
{
    for(uint i=0; i< 3; ++i)
    {
        if(shadow_coord[i].x >= 0.0 && shadow_coord[i].x < 1.0 && 
           shadow_coord[i].y >= 0.0 && shadow_coord[i].y < 1.0 && 
           shadow_coord[i].z >= 0.0 && shadow_coord[i].z < 1.0)
        {

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

    return color;
}

--- NO_SHADOW

vec3 ComputeShadow(in vec3 color)
{
    return color;
}

