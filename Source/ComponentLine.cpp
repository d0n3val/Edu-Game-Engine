#include "Globals.h"

#include "ComponentLine.h"
#include "Application.h"
#include "ModuleResources.h"

#include "OGL.h"
#include "OpenGL.h"

namespace
{
    VertexAttrib attribs[]   = { {0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0 }, 
                                 {1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0 } };
                                 
    const float positions[]   = { -0.5f,  -0.5f, 0.0f , 
                                   0.5f,  -0.5f, 0.0f , 
                                   0.5f,   0.5f, 0.0f , 
                                   -0.5f,  0.5f, 0.0f };
                                  
    const float texCoord[]   = { 0.0f,  0.0f, 
                                 1.0f,  0.0f, 
                                 1.0f,  1.0f, 
                                 0.0f,  1.0f };
                                  

    const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

}

ComponentLine::ComponentLine(GameObject* go) : Component(go, Types::Line)
{
    Buffer* vbo_ptr[ATTRIB_COUNT];
    vbo_ptr[ATTRIB_POS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(positions), positions);
    vbo_ptr[ATTRIB_TEXCOORD] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(texCoord), texCoord);

    for(uint i=0; i<ATTRIB_COUNT; ++i) vbo[i].reset(vbo_ptr[i]);


    ebo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), indices));

    vao = std::make_unique<VertexArray>(vbo_ptr, ebo.get(), attribs, ATTRIB_COUNT);
}

ComponentLine::~ComponentLine()
{
}

void ComponentLine::OnPlay() 
{

}

void ComponentLine::OnStop() 
{

}

void ComponentLine::OnUpdate(float dt) 
{
}

void ComponentLine::OnSave(Config &config) const 
{
	config.AddUID("Texture", texture.GetUID());
}

void ComponentLine::OnLoad(Config *config) 
{
    texture = config->GetUID("Texture", 0);
}
