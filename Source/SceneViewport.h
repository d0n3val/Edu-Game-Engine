#ifndef _SCENE_VIEWPORT_H_
#define _SCENE_VIEWPORT_H_

#include "Math.h"
#include "ImGuizmo.h"

class ComponentCamera;
class GameObject;
class PointLight;
class SpotLight;
class Config;

class SceneViewport
{
public:
    SceneViewport();
    ~SceneViewport();

    void                Draw                (ComponentCamera* camera);

	void                Save                (Config* config) const;
	void                Load                (Config* config);


    unsigned            GetWidth            () const { return fb_width; }
    unsigned            GetHeight           () const { return fb_height; }
    unsigned            GetXPos             () const { return x_pos; }
    unsigned            GetYPos             () const { return y_pos; }

    bool                IsFocused           () const { return focused; }
	bool                IsUsingGuizmo       () const { return ImGuizmo::IsUsing(); }

    void                DrawGuizmoProperties(GameObject* go);
    void                DrawGuizmoProperties(PointLight* point);
    void                DrawGuizmoProperties(SpotLight* spot);


private:

    struct Framebuffer;

    void GenerateFBOs(unsigned w, unsigned h);
    void GenerateFBO(Framebuffer& buffer, unsigned w, unsigned h, bool depth, bool msaa, bool hdr);
    void DrawQuickBar(ComponentCamera* camera);
    void DrawGuizmo(ComponentCamera* camera);
    void DrawGuizmo(ComponentCamera* camera, GameObject* go);
    void DrawGuizmo(ComponentCamera* camera, PointLight* point);
    void DrawGuizmo(ComponentCamera* camera, SpotLight* spot);
    float DistanceFromAtt(float constant, float linear, float quadric, float epsilon);
    void RemoveFrameBuffer(Framebuffer& buffer);

private:

    struct Framebuffer
    {
        unsigned id    = 0;
        unsigned depth = 0;
        unsigned tex   = 0;
    };

    Framebuffer fbuffer;
    Framebuffer msaa_fbuffer;
    Framebuffer post_fbuffer;

    unsigned    fb_width    = 0;
    unsigned    fb_height   = 0;

    unsigned    x_pos       = 0;
    unsigned    y_pos       = 0;
    bool        active      = true;
    bool        focused     = false;
	bool        draw_plane  = true;
	bool        draw_axis   = true;
	bool        debug_draw  = true;

    ImGuizmo::OPERATION guizmo_op      = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      guizmo_mode    = ImGuizmo::WORLD;
    bool                guizmo_useSnap = false;
    float3              guizmo_snap    = float3(1.0f);

};

#endif /* _VIEWPORT_H_ */

