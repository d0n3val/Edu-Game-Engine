struct Material
{
    sampler2D diffuse_map;
};

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    float lambda;
    vec3 position;
    vec4 color;
};

layout(location=0) uniform Material material;


//////////////////// INPUTS ////////////////////////

in VertexOut fragment;

//////////////////// OUTPUT ////////////////////////

out vec4 color;

void main()
{
	vec4 diffuse0 = texture(material.diffuse_map, fragment.uv0);
	vec4 diffuse1 = texture(material.diffuse_map, fragment.uv1);
    vec4 diffuse  = diffuse0*fragment.color;
    //vec4 diffuse  = mix(diffuse0, diffuse1, fragment.lambda)*fragment.color;

    color = diffuse;

	// gamma correction
    color.rgb   = pow(color.rgb, vec3(2.2));
}
