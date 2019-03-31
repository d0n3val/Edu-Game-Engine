struct VertexOut
{
    vec4 color;
};

out vec4 color;

in VertexOut fragment;

void main()
{
    color = fragment.color;

	// gamma correction
    color.rgb   = pow(color.rgb, vec3(2.2));
}
