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

    void TestDraw();
    void DrawNodes();
    void ShowContextMenus(ComponentAnimation* animation);
    void ShowCreateNewNode(ComponentAnimation* animation);
    void AddAnimationNode(ComponentAnimation* animation, uint index);

private:
    ax::NodeEditor::EditorContext* context = nullptr;

};


#endif /* _STATE_VIEWPORT_H_ */
