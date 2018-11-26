#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"

class ComponentLight : public Component
{
public:
    explicit ComponentLight(GameObject* container);
    ~ComponentLight();

    virtual void OnSave(Config& config) const override;
    virtual void OnLoad(Config* config) override;

    static Types GetClassType() { return Light; }
public:

    enum Type
    {
        POINT,
        DIRECTIONAL
    };

    Type type = POINT;
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
};

#endif /* __COMPONENT_LIGHT_H__ */


