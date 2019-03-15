#include "Globals.h"

#include "Billboard.h"
#include "ComponentCamera.h"

Billboard::Billboard()
{
}

Billboard::~Billboard()
{
}

void Billboard::GetVertices(float3* quad, const ComponentCamera* camera) const
{
	float3 front = position - camera->frustum.pos;
	front.Normalize();
	float3 right = front.Cross(float3::unitY);
	right.Normalize();

	quad[0] = position-right*half_size.x-float3::unitY*half_size.y;
	quad[1] = position+right*half_size.x-float3::unitY*half_size.y;
	quad[2] = position+right*half_size.x+float3::unitY*half_size.y;
	quad[3] = position-right*half_size.x+float3::unitY*half_size.y;
}

void Billboard::GetVertices(const Billboard* billboards, uint count, float3* quads, const ComponentCamera* camera)
{
    for(uint i=0; i< count; ++i)
    {
        billboards[i].GetVertices(&quads[i*4], camera);
    }
}

