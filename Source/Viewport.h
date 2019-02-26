#ifndef _VIEWPORT_H_
#define _VIEWPORT_H_

#include "Math.h"
#include "ImGuizmo.h"

class ComponentCamera;
class Config;
class SceneViewport;
class StateViewport;

class Viewport
{
public:
    Viewport();
    ~Viewport();

    void                    Draw        (ComponentCamera* camera);

	void                    Save        (Config* config) const;
	void                    Load        (Config* config);

    const SceneViewport*    GetScene    () const { return scene; }
    SceneViewport*          GetScene    () { return scene; }

private:

    SceneViewport* scene;
    StateViewport* state;

};

#endif /* _VIEWPORT_H_ */
