#ifndef _STATE_VIEWPORT_H_
#define _STATE_VIEWPORT_H_

#include "NodeEditor.h"

namespace ed = ax::NodeEditor;

class ComponentAnimation;
class ResourceStateMachine;

class StateViewport
{
public:
    StateViewport();
    ~StateViewport();

    void Draw  (ResourceStateMachine* animation, ax::NodeEditor::EditorContext* context);

private:

    void DrawNodes(ResourceStateMachine* animation);
    void DrawTransitions(ResourceStateMachine* animation);
    void ShowContextMenus(ResourceStateMachine* animation);
    void ShowCreateNewNodeMenu(ResourceStateMachine* animation);
    void ShowNodeMenu(ResourceStateMachine* animation);
    void ShowLinkMenu(ResourceStateMachine* animation);
    void AddAnimationNode(ResourceStateMachine* animation, uint index);
    void ManageCreate(ResourceStateMachine* animation);

    static bool GetClip(void* data, int idx, const char** out_text);
    static bool GetNode(void* data, int idx, const char** out_text);

private:
    uint context_node = 0;
    uint context_link = 0;
    ImVec2 new_node_pos;
    ed::PinId new_node_pin = 0;
};


#endif /* _STATE_VIEWPORT_H_ */
