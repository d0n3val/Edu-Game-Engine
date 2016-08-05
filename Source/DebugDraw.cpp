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
	float3 pos = aabb.CenterPoint();
	glColor3f(color.r, color.g, color.b);

	float3 size = aabb.HalfSize();
	float sx = size.x;
	float sy = size.y;
	float sz = size.z;

	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-sx, -sy, sz);
	glVertex3f( sx, -sy, sz);
	glVertex3f( sx,  sy, sz);
	glVertex3f(-sx,  sy, sz);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f( sx, -sy, -sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx,  sy, -sz);
	glVertex3f( sx,  sy, -sz);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(sx, -sy,  sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(sx,  sy, -sz);
	glVertex3f(sx,  sy,  sz);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, -sy,  sz);
	glVertex3f(-sx,  sy,  sz);
	glVertex3f(-sx,  sy, -sz);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-sx, sy,  sz);
	glVertex3f( sx, sy,  sz);
	glVertex3f( sx, sy, -sz);
	glVertex3f(-sx, sy, -sz);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f( sx, -sy, -sz);
	glVertex3f( sx, -sy,  sz);
	glVertex3f(-sx, -sy,  sz);

	glEnd();
}

void DebugDraw(const OBB & obb, Color color)
{
	float3 pos = obb.CenterPoint();
	glColor3f(color.r, color.g, color.b);

	float3 vertex[36];
	obb.Triangulate(1, 1, 1, vertex, nullptr, nullptr, true);

	/*
	glBegin(GL_TRIANGLES);

	for (int i = 0; i < 36; ++i)
		glVertex3fv((GLfloat *) &vertex[i]);

	glEnd();
	  */

	/* The points are returned in the order 
	0: ---, 1: --+, 2: -+-, 3: -++, 4: +--, 5: +-+, 6: ++-, 7: +++.
	*/

	float3 corners[8];
	obb.GetCornerPoints(corners);

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
