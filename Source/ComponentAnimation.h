#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "HashString.h"

#include <vector>
#include <list>

class AnimController;

class ComponentAnimation : public Component
{
public:

	ComponentAnimation (GameObject* container);
	~ComponentAnimation ();

	virtual void        OnPlay              () override;
	virtual void        OnStop              () override;
	virtual void        OnUpdate            (float dt) override;

	virtual void        OnSave              (Config& config) const override;
	virtual void        OnLoad              (Config* config) override;

    void                AddClip             (const HashString& name, UID resource, bool loop);
    void                RemoveClip          (uint index);
    uint                FindClip            (const HashString& name) const;

    uint                GetNumClips         () const           { return clips.size(); }
    const HashString&   GetClipName         (uint index) const { return clips[index].name; }
    UID                 GetClipRes          (uint index) const { return clips[index].resource; }
    bool                GetClipLoop         (uint index) const { return clips[index].loop; }

    void                SetClipName         (uint index, const HashString& name) { clips[index].name = name; }
    void                SetClipRes          (uint index, UID uid)                { clips[index].resource = uid; }
    void                SetClipLoop         (uint index, bool loop)              { clips[index].loop = loop; }

    uint                GetNumNodes         () const           { return nodes.size(); }
    const HashString&   GetNodeName         (uint index) const { return nodes[index].name; }
    const HashString&   GetNodeClip         (uint index) const { return nodes[index].clip; }
    float               GetNodeSpeed        (uint index) const { return nodes[index].speed; }

    void                AddNode             (const HashString& name, const HashString& clip, float speed);
    void                RemoveNode          (uint index);
    uint                FindNode            (const HashString& name) const;

    void                SetNodeName         (uint index, const HashString& name) { nodes[index].name = name; }
    void                SetNodeClip         (uint index, const HashString& clip) { nodes[index].clip = clip; }
    void                SetNodeSpeed        (uint index, float speed) { nodes[index].speed = speed; }

    uint                GetNumTransitions   () const { return transitions.size();}
    const HashString&   GetTransitionSource (uint index) const { return transitions[index].source;}
    const HashString&   GetTransitionTarget (uint index) const { return transitions[index].target;}
    const HashString&   GetTransitionTrigger(uint index) const { return transitions[index].trigger;}
    uint                GetTransitionBlend  (uint index) const { return transitions[index].blend; }

    void                AddTransition       (const HashString& source, const HashString& target, const HashString& trigger, uint blend);
    void                RemoveTransition    (uint index);
    uint                FindTransition      (const HashString& source, const HashString& trigger) const;

    void                SetTransitionSource (uint index, const HashString& source) { transitions[index].source = source; }
    void                SetTransitionTarget (uint index, const HashString& target) { transitions[index].target = target; }
    void                SetTransitionTrigger(uint index, const HashString& trigger) { transitions[index].trigger = trigger; }
    void                SetTransitionBlend  (uint index, uint blend) { transitions[index].blend = blend; }

    bool                GetDebugDraw        () const {return debug_draw;}
    void                SetDebugDraw        (bool enable) { debug_draw = enable; }

    static Types        GetClassType        () { return Animation; }

private:

    void                UpdateGO                (GameObject* go);
    void                RemoveNodeTransitions   (const HashString& name);

private:

    struct Clip
    {
        HashString      name;
        UID             resource = 0;
        bool            loop     = false;
        std::list<uint> frame_tags;

        Clip() {;}
        Clip(const HashString& n, UID r, bool l) : name(n), resource(r), loop(l) {;}
    };

    struct Transition
    {
        HashString source;
        HashString target;
        HashString trigger;
        unsigned   blend = 200;

        Transition() {;}
        Transition(const HashString& s, const HashString& t, const HashString& tr, uint b) : source(s), target(t), trigger(tr), blend(b) {;}
    };

    struct Node
    {
        HashString name;
        HashString clip;
        float      speed = 1.0;

        Node() {;}
        Node(const HashString& n, const HashString& c, float s) : name(n), clip(c), speed(s) {;}
    };

    AnimController*         controller = nullptr;
    std::vector<Clip>       clips;
    std::vector<Node>       nodes;
    std::vector<Transition> transitions;
    unsigned                active_node = 0;
    bool                    debug_draw  = false;
};

#endif // __COMPONENT_AUDIOSOURCE_H__
