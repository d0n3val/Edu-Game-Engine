#ifndef _VIEWPORT_H_
#define _VIEWPORT_H_

#include "Math.h"
#include "ImGuizmo.h"

class ComponentCamera;
class GameObject;

class Viewport
{
public:
    Viewport();
    ~Viewport();

    void                Draw                (ComponentCamera* camera);

    unsigned            GetWidth            () const { return fb_width; }
    unsigned            GetHeight           () const { return fb_height; }

    bool                IsFocused           () const { return focused; }
	bool                IsUsingGuizmo       () const { return ImGuizmo::IsUsing(); }

    void                DrawGuizmoProperties(GameObject* go);

private:

    void GenerateFBOTexture(unsigned w, unsigned h);
    void DrawQuickBar();
    void DrawGuizmo(ComponentCamera* camera);

private:

    unsigned fbo         = 0;
    unsigned fb_depth    = 0;
    unsigned fb_tex      = 0;
    unsigned fb_width    = 0;
    unsigned fb_height   = 0;
    bool     active      = true;
    bool     focused     = false;

    ImGuizmo::OPERATION guizmo_op      = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      guizmo_mode    = ImGuizmo::WORLD;
    bool                guizmo_useSnap = false;
    float3              guizmo_snap    = float3(1.0f);

};

#endif /* _VIEWPORT_H_ */
