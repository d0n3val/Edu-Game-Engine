struct Material
{
    sampler2D diffuse_map;
};

layout(location=0) uniform Material material;

struct VertexOut
{
    vec2 uv;
    vec4 color;
};

out vec4 color;

in VertexOut fragment;

void main()
{
	vec4 diffuse= texture(material.diffuse_map, fragment.uv);
    color = diffuse*fragment.color;

	// gamma correction
    color.rgb   = pow(color.rgb, vec3(2.2));
}
