#ifndef _VIEWPORT_H_
#define _VIEWPORT_H_

#include "Math.h"
#include "ImGuizmo.h"

class ComponentCamera;
class GameObject;
class PointLight;
class Config;

class Viewport
{
public:
    Viewport();
    ~Viewport();

    void                Draw                (ComponentCamera* camera);

	void                Save                (Config* config) const;
	void                Load                (Config* config);

    unsigned            GetFrameBuffer      () const { return fbo; }
    unsigned            GetWidth            () const { return fb_width; }
    unsigned            GetHeight           () const { return fb_height; }
    unsigned            GetXPos             () const { return x_pos; }
    unsigned            GetYPos             () const { return y_pos; }

    bool                IsFocused           () const { return focused; }
	bool                IsUsingGuizmo       () const { return ImGuizmo::IsUsing(); }

    void                DrawGuizmoProperties(GameObject* go);
    void                DrawGuizmoProperties(PointLight* point);
    void                DrawGuizmoProperties(SpotLight* point);

private:

    void GenerateFBOTexture(unsigned w, unsigned h);
    void GenerateFBOMultisampled(unsigned w, unsigned h);
    void DrawQuickBar(ComponentCamera* camera);
    void DrawGuizmo(ComponentCamera* camera);

private:

    unsigned fbo         = 0;
    unsigned msfbo       = 0;
    unsigned fb_depth    = 0;
    unsigned msfb_depth  = 0;
    unsigned msfb_color  = 0;
    unsigned fb_tex      = 0;
    unsigned fb_width    = 0;
    unsigned fb_height   = 0;
    unsigned x_pos       = 0;
    unsigned y_pos       = 0;
    bool     active      = true;
    bool     focused     = false;
    bool     msaa        = true;
	bool     draw_plane  = true;
	bool     draw_axis   = true;
	bool     debug_draw  = true;

    ImGuizmo::OPERATION guizmo_op      = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      guizmo_mode    = ImGuizmo::WORLD;
    bool                guizmo_useSnap = false;
    float3              guizmo_snap    = float3(1.0f);

};

#endif /* _VIEWPORT_H_ */
