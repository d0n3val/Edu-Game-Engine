
out vec4 color;

void main()
{
    color = vec4(1.0);

	// gamma correction
    color.rgb   = pow(color.rgb, vec3(2.2));
}
