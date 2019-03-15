#ifndef __BILLBOARD_H__
#define __BILLBOARD_H__

#include "Math.h"

class ComponentCamera;

class Billboard
{
public:

	Billboard();
	~Billboard();

	void            GetVertices (float3* quad, const ComponentCamera* camera) const;
    static void     GetVertices (const Billboard* billboards, uint count, float3* quads, const ComponentCamera* camera);

	void            SetPosition (const float3& pos) { position = pos; }
	const float3&   GetPosition () const { return position; }

	void            SetSize     (const float2& half) { half_size = half; }
	const float2&   GetSize     () const { return half_size; }

private:
    
	float3 position  = float3::zero;
	float2 half_size = float2::zero;
};


#endif /* __BILLBOARD_H__ */
