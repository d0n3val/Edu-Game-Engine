struct Material
{
    sampler2D diffuse_map;
};

struct VertexOut
{
    vec2 uv0;
    vec3 position;
};

layout(location=0) uniform Material material;


//////////////////// INPUTS ////////////////////////

in VertexOut fragment;

//////////////////// OUTPUT ////////////////////////

out vec4 color;

void main()
{
	vec4 diffuse = texture(material.diffuse_map, fragment.uv0);

    color = vec4(diffuse.rgb, 1.0);

	// gamma correction
    //color.rgb   = pow(color.rgb, vec3(1.0/2.2));
}
