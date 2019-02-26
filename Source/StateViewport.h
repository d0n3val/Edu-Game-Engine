#ifndef _STATE_VIEWPORT_H_
#define _STATE_VIEWPORT_H_

namespace ax { namespace NodeEditor { struct EditorContext; } }

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
    void ShowContextMenus(ComponentAnimation* animation, uint& node_idx, uint& link_idx);
    void ShowCreateNewNodeMenu(ComponentAnimation* animation);
    void ShowNodeMenu(ComponentAnimation* animation, uint node_idx);
    void ShowLinkMenu(ComponentAnimation* animation, uint node_idx);
    void AddAnimationNode(ComponentAnimation* animation, uint index);
    void ManageCreate(ComponentAnimation* animation);

private:
    ax::NodeEditor::EditorContext* context = nullptr;

};


#endif /* _STATE_VIEWPORT_H_ */
