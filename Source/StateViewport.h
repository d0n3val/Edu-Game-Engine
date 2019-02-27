#ifndef _STATE_VIEWPORT_H_
#define _STATE_VIEWPORT_H_

#include "NodeEditor.h"

namespace ed = ax::NodeEditor;

class ComponentAnimation;

class StateViewport
{
public:
    StateViewport();
    ~StateViewport();

    void Draw  (ComponentAnimation* animation);

private:

    void DrawNodes(ComponentAnimation* animation);
    void DrawTransitions(ComponentAnimation* animation);
    void ShowContextMenus(ComponentAnimation* animation);
    void ShowCreateNewNodeMenu(ComponentAnimation* animation);
    void ShowNodeMenu(ComponentAnimation* animation);
    void ShowLinkMenu(ComponentAnimation* animation);
    void AddAnimationNode(ComponentAnimation* animation, uint index);
    void ManageCreate(ComponentAnimation* animation);

    static bool GetClip(void* data, int idx, const char** out_text);
    static bool GetNode(void* data, int idx, const char** out_text);

private:
    ed::EditorContext* context = nullptr;
    uint context_node = 0;
    uint context_link = 0;
    ImVec2 new_node_pos;
    ed::PinId new_node_pin = 0;
};


#endif /* _STATE_VIEWPORT_H_ */
