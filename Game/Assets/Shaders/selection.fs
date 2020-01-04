layout(location=1) uniform float id;

out vec4 color;

void main()
{
    color = vec4(id, 0.0, 0.0, 1.0);
}
