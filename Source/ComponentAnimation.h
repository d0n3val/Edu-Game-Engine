#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "HashString.h"

#include <vector>

class AnimController;

class ComponentAnimation : public Component
{
public:

	ComponentAnimation (GameObject* container);
	~ComponentAnimation ();

	virtual void        OnStart     () override;
	virtual void        OnUpdate    (float dt) override;
	virtual void        OnFinish    () override;

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
        HashString  name;
        UID         resource = 0;
        bool        loop     = false;

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

    AnimController*   controller = nullptr;
    std::vector<Clip> clips;
    bool              debug_draw = false;
};

#endif // __COMPONENT_AUDIOSOURCE_H__
