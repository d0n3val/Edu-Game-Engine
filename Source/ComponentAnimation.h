#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "HashString.h"

#include <vector>
#include <list>

class AnimController;
class ResourceStateMachine;

class ComponentAnimation : public Component
{
public:

	ComponentAnimation (GameObject* container);
	~ComponentAnimation ();

	virtual void                OnPlay      () override;
	virtual void                OnStop      () override;
	virtual void                OnUpdate    (float dt) override;

	virtual void                OnSave      (Config& config) const override;
	virtual void                OnLoad      (Config* config) override;

	bool                        SetResource (UID uid);
    const ResourceStateMachine* GetResource () const;
    ResourceStateMachine*       GetResource ();

    bool                        GetDebugDraw() const {return debug_draw;}
    void                        SetDebugDraw(bool enable) { debug_draw = enable; }

    static Types                GetClassType() { return Animation; }

private:

    void                UpdateGO                (GameObject* go);

private:

    UID                     resource   = 0;
    AnimController*         controller = nullptr;
    unsigned                active_node = 0;
    bool                    debug_draw  = false;
};

#endif // __COMPONENT_AUDIOSOURCE_H__
