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
	glDisable(GL_DEPTH_TEST);
	glColor4f(1.f, 1.f, 1.f, 1.f);
}

// ------------------------------------------------------------
void EndDebugDraw()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}


 /*scalex - scaling of sphere around x-axis
   scaley - scaling of sphere around y-axis
   r - radius of sphere
  */
 void drawHalfSphere(GLfloat r) {
   int i, j;
   int scalex, scaley;
   scalex = scaley = 12;
   GLfloat v[12*12][3];
 
   for (i=0; i<scalex; ++i) {
     for (j=0; j<scaley; ++j) {
       v[i*scaley+j][0]=r*cos(j*2*PI/scaley)*cos(i*PI/(2*scalex));
       v[i*scaley+j][1]=r*sin(i*PI/(2*scalex));
       v[i*scaley+j][2]=r*sin(j*2*PI/scaley)*cos(i*PI/(2*scalex));
     }
   }
 
   glBegin(GL_QUADS);
     for (i=0; i<scalex-1; ++i) {
       for (j=0; j<scaley; ++j) {
         glVertex3fv(v[i*scaley+j]);
         glVertex3fv(v[i*scaley+(j+1)%scaley]);
         glVertex3fv(v[(i+1)*scaley+(j+1)%scaley]);
         glVertex3fv(v[(i+1)*scaley+j]);
       }
     }
   glEnd();
 }

// ------------------------------------------------------------
void DebugDraw(const Sphere & sphere, Color color, const float4x4 & transform)
{
	glColor3f(color.r, color.g, color.b);

	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());

	int stacks = 10;
	int slices = 10;

    int i,j;
    for (j = 0; j < stacks; j++) {
        double latitude1 = (PI/stacks) * j - PI/2;
        double latitude2 = (PI/stacks) * (j+1) - PI/2;
        double sinLat1 = sin(latitude1);
        double cosLat1 = cos(latitude1);
        double sinLat2 = sin(latitude2);
        double cosLat2 = cos(latitude2);
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= slices; i++) {
            double longitude = (2*PI/slices) * i;
            double sinLong = sin(longitude);
            double cosLong = cos(longitude);
            double x1 = cosLong * cosLat1;
            double y1 = sinLong * cosLat1;
            double z1 = sinLat1;
            double x2 = cosLong * cosLat2;
            double y2 = sinLong * cosLat2;
            double z2 = sinLat2;
            glNormal3d(x2,y2,z2);
            glVertex3d(sphere.r*x2,sphere.r*y2,sphere.r*z2);
            glNormal3d(x1,y1,z1);
            glVertex3d(sphere.r*x1,sphere.r*y1,sphere.r*z1);
        }
        glEnd();
    }
	glPopMatrix();
}

// ------------------------------------------------------------
void DebugDraw(const AABB & aabb, Color color, const float4x4& transform)
{
	static float3 corners[8];
	aabb.GetCornerPoints(corners);

	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());
	DebugDrawBox(corners, color);
	glPopMatrix();
}

// ------------------------------------------------------------
void DebugDraw(const OBB& obb, Color color, const float4x4& transform)
{
	static float3 corners[8];
	obb.GetCornerPoints(corners);

	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());
	DebugDrawBox(corners, color);
	glPopMatrix();
}

// ------------------------------------------------------------
void DebugDraw(const Capsule & capsule, Color color, const float4x4 & transform)
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());

	int n = 12;
	float height = capsule.LineLength();
	float radius = capsule.r;

	// Cap 1
	glPushMatrix();
	glTranslatef(0.f, height*0.5f, 0.f);
	drawHalfSphere(capsule.r);
	glPopMatrix();

	// Cap 2
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	glPushMatrix();
	glTranslatef(0.f, height*0.5f, 0.f);
	drawHalfSphere(capsule.r);
	glPopMatrix();
	glPopMatrix();

	// Cover between caps
	glPushMatrix();
	glRotatef(90, 0, 0, 1);
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians

		glVertex3f(height*0.5f,  radius * cosf(a), radius * sinf(a) );
		glVertex3f(-height*0.5f, radius * cosf(a), radius * sinf(a) );
	}
	glEnd();

	glPopMatrix();
	glPopMatrix();
}

// ------------------------------------------------------------
// Draw Axis
void DebugDraw(const float4x4 & transform)
{
	// TODO scale based on distance to keep same screen space ?
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());
	glLineWidth(2.0f);

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
	glLineWidth(1.0f);
	glPopMatrix();
}

void DebugDraw(const Frustum & frustum, Color color)
{
	float3 vertices[8];
	frustum.GetCornerPoints(vertices);

	DebugDrawBox(vertices, color);
}

void DebugDraw(const LineSegment & segment, Color color, bool is_arrow, const float4x4& transform)
{
	glColor3f(color.r, color.g, color.b);
	glLineWidth(2.0f);

	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());

	glBegin(GL_LINES);

	glVertex3fv((GLfloat*)&segment.a);
	glVertex3fv((GLfloat*)&segment.b);
	
	glEnd();

	glPopMatrix();
	glLineWidth(1.0f);
}

void DebugDraw(const Ray & ray, Color color, float max_dist)
{
	glColor3f(color.r, color.g, color.b);
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glVertex3fv((GLfloat*)&ray.pos);
	glVertex3fv((GLfloat*)&ray.GetPoint(max_dist));

	glEnd();
	glLineWidth(1.0f);
}

void DebugDraw(const float3 & point, Color color)
{
	glColor3f(color.r, color.g, color.b);
	glPointSize(3.0f);
	glBegin(GL_POINTS);

	glVertex3fv((GLfloat*)&point);

	glEnd();
	glPointSize(1.0f);
}

void DebugDraw(const Cylinder & cylinder, Color color, const float4x4& transform)
{
	int n = 20;
	float height = cylinder.Height();
	float radius = cylinder.r;

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	
	for(int i = 360; i >= 0; i -= (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians
		glVertex3f(-height*0.5f, radius * cosf(a), radius * sinf(a));
	}
	glEnd();

	// Cylinder Top
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for(int i = 0; i <= 360; i += (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians
		glVertex3f(height * 0.5f, radius * cosf(a), radius * sinf(a));
	}
	glEnd();

	// Cylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians

		glVertex3f(height*0.5f,  radius * cosf(a), radius * sinf(a) );
		glVertex3f(-height*0.5f, radius * cosf(a), radius * sinf(a) );
	}
	glEnd();
	glPopMatrix();
}

void DebugDraw(const Cone & cone, Color color, const float4x4 & transform)
{
	int n = 20;
	float height = cone.Height();
	float radius = cone.r;

	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	
	for(int i = 360; i >= 0; i -= (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians
		glVertex3f(-height*0.5f, radius * cosf(a), radius * sinf(a));
	}
	glEnd();

	// Cylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = (float)i * PI / 180.f; // degrees to radians

		glVertex3f(height*0.5f,  0.f, 0.f );
		glVertex3f(-height*0.5f, radius * cosf(a), radius * sinf(a) );
	}
	glEnd();
	glPopMatrix();
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

void DebugDrawArrowZ(const float3& offset, float length, Color color, const float4x4 & transform)
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());
	glLineWidth(3.0f);
	glBegin(GL_LINES);

	glVertex3fv((GLfloat*)&offset.x);
	float3 dest = offset + (float3::unitZ * length);
	glVertex3fv((GLfloat*)&dest.x);

	float arrow_head_size = 0.2f;
	float3 side, up;

	side = float3(offset.x + arrow_head_size, offset.y, offset.z + (length - arrow_head_size));
	glVertex3fv((GLfloat*)&dest.x);
	glVertex3fv((GLfloat*)&side.x);

	side = float3(offset.x -arrow_head_size, offset.y, offset.z + (length - arrow_head_size));
	glVertex3fv((GLfloat*)&dest.x);
	glVertex3fv((GLfloat*)&side.x);

	glEnd();
	glLineWidth(1.0f);
	glPopMatrix();
}

void DebugDrawArrow(const float3& dir, const float3& offset, Color color, const float4x4 & transform)
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf((GLfloat*) transform.Transposed().ptr());
	glLineWidth(3.0f);
	glBegin(GL_LINES);

	glVertex3fv((GLfloat*)&offset.x);
	float3 dest = offset + dir;
	glVertex3fv((GLfloat*)&dest.x);

	float arrow_head_size = 0.2f;
	float length = dir.Length();
	float3 side, up;
	float angle = dir.AngleBetween(float3::unitZ);
	//LOG("%f", angle);
	Quat q(float3::unitY, angle);
	
	side = q* float3(offset.x + arrow_head_size, offset.y, offset.z + (length - arrow_head_size));

	glVertex3fv((GLfloat*)&dest.x);
	glVertex3fv((GLfloat*)&side.x);

	side = q* float3(offset.x -arrow_head_size, offset.y, offset.z + (length - arrow_head_size));
	glVertex3fv((GLfloat*)&dest.x);
	glVertex3fv((GLfloat*)&side.x);

	glEnd();
	glLineWidth(1.0f);
	glPopMatrix();
}
