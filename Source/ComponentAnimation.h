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

	virtual void        OnPlay      () override;
	virtual void        OnStop      () override;
	virtual void        OnUpdate    (float dt) override;

	virtual void        OnSave      (Config& config) const override;
	virtual void        OnLoad      (Config* config) override;

    void                AddClip     (const HashString& name, UID resource, bool loop);
    uint                FindClip    (const HashString& name) const;

    uint                GetNumClips () const { return clips.size(); }
    const HashString&   GetClipName (uint index) const { return clips[index].name; }
    UID                 GetClipRes  (uint index) const { return clips[index].resource; }
    bool                GetClipLoop (uint index) const { return clips[index].loop; }

    void                SetClipRes  (uint index, UID uid) { clips[index].resource = uid; }
    void                SetClipLoop (uint index, bool loop) { clips[index].loop = loop; }

    bool                GetDebugDraw () const {return debug_draw;}
    void                SetDebugDraw (bool enable) { debug_draw = enable; }

    static Types        GetClassType() { return Animation; }

private:

    void                UpdateGO    (GameObject* go);

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

    struct TNearestClip
    {
        bool operator()(const Clip& clip, const HashString& name)
        {
            return clip.name < name;
        }
    };

    struct Transition
    {
        HashString trigger;
        HashString target_node;
        unsigned   blend_time = 200;
    };

    struct Node
    {
        HashString            name;
        HashString            clip;
        float                 speed = 1.0;
        std::list<Transition> transitions;
    };

    AnimController*   controller = nullptr;
    std::vector<Clip> clips;
    std::vector<Node> nodes;
    unsigned          current    = 0;
    bool              debug_draw = false;
};

#endif // __COMPONENT_AUDIOSOURCE_H__
