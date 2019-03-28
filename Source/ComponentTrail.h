#ifndef __COMPONENT_TRAIL_H_
#define __COMPONENT_TRAIL_H_

#include "Component.h"

class ComponentTrail : public Component
{
public:
    ComponentTrail(GameObject* go);
    ~ComponentTrail();

	void         OnPlay       () override;
	void         OnStop       () override;
	void         OnUpdate     (float dt) override;

	void         OnSave       (Config& config) const override;
	void         OnLoad       (Config* config) override;

    static Types GetClassType () { return Trail; }

};

#endif /* __COMPONENT_TRAIL_H_ */
