#define SPECULAR_COLOR vec4(0.3, 0.3, 0.3, 1.0)
#define AMBIENT_AMOUNT 0.3

in VS_OUT
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
} fs_in;


uniform sampler2D diffuse;
uniform sampler2D normal_map;

#if SPECULAR_MAP
uniform sampler2D specular_map;
#endif

#if RECEIVE_SHADOWS
uniform sampler2D shadow_map;
uniform float shadow_bias;
#endif

uniform float shininess;

out vec4 frag_color;

void main()
{

#if NORMAL_MAP
    vec3 normal           = normalize(texture2D(normal_map, fs_in.texcoord0).xyz*2.0-1.0);
#else
    vec3 normal           = fs_in.normal;
#endif

    float len             = length(normal);

#if LIGHT_DIRECTIONAL
    vec3 light_dir        = normalize(-fs_in.light_vector);
#else
    vec3 light_dir        = normalize(fs_in.light_vector-fs_in.frag_pos);
#endif

	float diffuse_amount  = max(0.0, dot(normal, light_dir)); 
	float specular_amount = 0.0;

	if(diffuse_amount != 0.0)
	{
        float ft             = len/mix(shininess, 1, len);
        float scale          = (1 + ft*shininess)/(1 + shininess);

        vec3 half_dir        = normalize(normalize(fs_in.camera_pos-fs_in.frag_pos)+light_dir);
		float half_intensity = max(0.0, dot(normal, half_dir)); 
		specular_amount      = scale*pow(half_intensity, ft*shininess);
	}

    vec4 diffuse_color = texture2D(diffuse, fs_in.texcoord0);

#if SPECULAR_MAP
    vec4 specular_color = texture2D(specular_map, fs_in.texcoord0);
#else
    vec4 specular_color = SPECULAR_COLOR;
#endif

	float visibility = 1.0;

#if RECEIVE_SHADOWS
    vec4 shadow_coord = (fs_in.shadow_coord/fs_in.shadow_coord.w)*0.5+0.5;
    float depth = clamp(shadow_coord.z, 0.0, 1.0);
    vec2 coord = vec2(shadow_coord.x, shadow_coord.y);

	if(coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0 &&
       texture2D(shadow_map, coord.xy).z < depth-shadow_bias)
	{
		visibility = 0.5;
	}
#endif

	frag_color = vec4(((AMBIENT_AMOUNT+diffuse_amount)*diffuse_color.rgb+
                     specular_color.rgb*specular_amount)*visibility, diffuse_color.a);
    frag_color = diffuse_color;
}


