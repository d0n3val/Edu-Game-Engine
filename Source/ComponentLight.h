#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"

class ComponentLight : public Component
{
public:
    ComponentLight();
    ~ComponentLight();

    virtual void OnSave(Config& config) const override;
    virtual void OnLoad(Config* config) override;

public:

    enum Type
    {
        LIGHT_POINT,
        LIGHT_DIRECTIONAL
    };

    Type        type      = LIGHT_POINT;
    float3      position  = float3(0.0f, 0.0f, 0.0f);
    float3      direction = float3(0.0f, 0.0f, 0.0f);
    float3      up        = float3(0.0f, 0.0f, 0.0f);
};

#endif /* __COMPONENT_LIGHT_H__ */


