#pragma once

#include "Component.h"

#include <memory>

#include "ResHandle.h"

class ResourceTexture;
class Buffer;
class VertexArray;

class ComponentLine : public Component
{
public:

    ComponentLine(GameObject* go);
    ~ComponentLine();

	void                    OnPlay          () override;
	void                    OnStop          () override;
	void                    OnUpdate        (float dt) override;

    void                    OnSave          (Config& config) const override;
    void                    OnLoad          (Config* config) override;

    const ResHandle&        GetTexture      () const { return texture; }
    void                    SetTexture      (UID tex) { texture = tex; }

    const VertexArray*      GetVAO          () const {return vao.get();}

private:

    friend void DrawLineComponent(ComponentLine* trail);

    enum Attribs
    {
        ATTRIB_POS = 0,
        ATTRIB_TEXCOORD,
        ATTRIB_COUNT
    };

    ResHandle texture;
    std::unique_ptr<Buffer> vbo[ATTRIB_COUNT];
    std::unique_ptr<Buffer> ebo;
    std::unique_ptr<VertexArray> vao;
};