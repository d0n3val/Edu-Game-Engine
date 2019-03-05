#ifndef __COMPONENT_SIMPLE_CHARACTER_H__
#define __COMPONENT_SIMPLE_CHARACTER_H__

#include "Component.h"

class ComponentSimpleCharacter: public Component
{
public:
    explicit ComponentSimpleCharacter(GameObject* go);
    ~ComponentSimpleCharacter();

    void OnSave     (Config& config) const override;
    void OnLoad     (Config* config) override;

	void OnUpdate   (float dt) override;
};

#endif /* __COMPONENT_SIMPLE_CHARACTER_H__ */
