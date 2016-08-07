#include "Globals.h"
#include "DebugDraw.h"
#include "OpenGL.h"
		
// ------------------------------------------------------------
void BeginDebugDraw()
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
}

// ------------------------------------------------------------
void EndDebugDraw()
{
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

// ------------------------------------------------------------
void DebugDraw(const AABB & aabb, Color color)
{
	static float3 corners[8];
	aabb.GetCornerPoints(corners);

	DebugDrawBox(corners, color);
}

// ------------------------------------------------------------
void DebugDraw(const OBB & obb, Color color)
{
	static float3 corners[8];
	obb.GetCornerPoints(corners);

	DebugDrawBox(corners, color);
}

// Draw Axis
void DebugDraw(const float4x4 & transform)
{
	// scale based on distance to keep same screen space ?
	static float4x4 m;
	
	m = transform.Transposed();

	glPushMatrix();
	glMultMatrixf((GLfloat*) m.v);
	glLineWidth(2.0f);

	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
	glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
	glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
	glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

	glEnd();
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0f);
	glPopMatrix();
}

void DebugDraw(const Frustum & frustum, Color color)
{
	float3 vertices[8];
	frustum.GetCornerPoints(vertices);

	DebugDrawBox(vertices, color);
}

void DebugDraw(const LineSegment & segment, Color color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINES);

	glLineWidth(3.0f);
	glVertex3fv((GLfloat*)&segment.a);
	glVertex3fv((GLfloat*)&segment.b);
	glLineWidth(1.0f);

	glEnd();
}

void DebugDraw(const float3 & point, Color color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_POINTS);

	glLineWidth(3.0f);
	glVertex3fv((GLfloat*)&point);
	glLineWidth(1.0f);

	glEnd();
}

void DebugDrawBox(const float3* corners, Color color)
{
	glColor3f(color.r, color.g, color.b);

	glBegin(GL_QUADS);

	glVertex3fv((GLfloat*) &corners[1]); //glVertex3f(-sx, -sy, sz);
	glVertex3fv((GLfloat*) &corners[5]); //glVertex3f( sx, -sy, sz);
	glVertex3fv((GLfloat*) &corners[7]); //glVertex3f( sx,  sy, sz);
	glVertex3fv((GLfloat*) &corners[3]); //glVertex3f(-sx,  sy, sz);

	glVertex3fv((GLfloat*) &corners[4]); //glVertex3f( sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[2]); //glVertex3f(-sx,  sy, -sz);
	glVertex3fv((GLfloat*) &corners[6]); //glVertex3f( sx,  sy, -sz);

	glVertex3fv((GLfloat*) &corners[5]); //glVertex3f(sx, -sy,  sz);
	glVertex3fv((GLfloat*) &corners[4]); //glVertex3f(sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[6]); //glVertex3f(sx,  sy, -sz);
	glVertex3fv((GLfloat*) &corners[7]); //glVertex3f(sx,  sy,  sz);

	glVertex3fv((GLfloat*) &corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[1]); //glVertex3f(-sx, -sy,  sz);
	glVertex3fv((GLfloat*) &corners[3]); //glVertex3f(-sx,  sy,  sz);
	glVertex3fv((GLfloat*) &corners[2]); //glVertex3f(-sx,  sy, -sz);

	glVertex3fv((GLfloat*) &corners[3]); //glVertex3f(-sx, sy,  sz);
	glVertex3fv((GLfloat*) &corners[7]); //glVertex3f( sx, sy,  sz);
	glVertex3fv((GLfloat*) &corners[6]); //glVertex3f( sx, sy, -sz);
	glVertex3fv((GLfloat*) &corners[2]); //glVertex3f(-sx, sy, -sz);

	glVertex3fv((GLfloat*) &corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[4]); //glVertex3f( sx, -sy, -sz);
	glVertex3fv((GLfloat*) &corners[5]); //glVertex3f( sx, -sy,  sz);
	glVertex3fv((GLfloat*) &corners[1]); //glVertex3f(-sx, -sy,  sz);

	glEnd();
}
