#include "Globals.h"
#include "StateViewport.h"
#include "ComponentAnimation.h"

#include "NodeEditor.h"

#include "mmgr/mmgr.h"

namespace ed = ax::NodeEditor;

StateViewport::StateViewport()
{
    ed::Config cfg;
    cfg.SettingsFile = "State.json";
    context = ed::CreateEditor(&cfg);
}

StateViewport::~StateViewport()
{
    ed::DestroyEditor(context);
}

void StateViewport::Draw(ComponentAnimation* animation)
{
    if(animation != nullptr)
    {
		ed::SetCurrentEditor(context);
		ed::Begin("State Machine Editor", ImVec2(0.0, 0.0f));

		DrawNodes(animation);
        DrawTransitions(animation);
        ManageCreate(animation);

        uint context_node = 0, context_link;
		ShowContextMenus(animation, context_node, context_link);

        ed::Suspend();
        ShowNodeMenu(animation, context_node);
        ShowLinkMenu(animation, context_link);
        ShowCreateNewNodeMenu(animation);
        ed::Resume();

		ed::End();
		ed::SetCurrentEditor(nullptr);
    }
}

void StateViewport::DrawNodes(ComponentAnimation* animation)
{
    for(uint i=0, count = animation->GetNumNodes(); i < count; ++i)
    {
        uint id = i+1;
        ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor( 60, 180, 255, 150));
        ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150));

        ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
        ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
        ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
        ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);

        ed::BeginNode(id);
        ImGui::Text(animation->GetNodeName(i).C_str());

        // In Pin
        ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
        ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
        ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
        ed::BeginPin(id*2, ed::PinKind::Input);
        ImGui::Text("In");
        ed::EndPin();
        ed::PopStyleVar(3);

        // Out Pin
        ImGui::SameLine();
        ed::BeginPin(id*2+1, ed::PinKind::Output);
        ImGui::Text("Out");
        ed::EndPin();
        ed::EndNode();
        ed::PopStyleColor(2);
        ed::PopStyleVar(5);
    }
}

void StateViewport::DrawTransitions(ComponentAnimation* animation)
{
    for(uint i=0, count = animation->GetNumTransitions(); i < count; ++i)
    {
        uint id     = i+1;
        uint source = animation->FindNode(animation->GetTransitionSource(i));
        uint target = animation->FindNode(animation->GetTransitionTarget(i));

        if(source < animation->GetNumNodes() && target < animation->GetNumNodes())
        {
            ed::Link(id, source*2, source*2+1);
        }
    }
}

void StateViewport::AddAnimationNode(ComponentAnimation* animation, uint index)
{
    HashString name = animation->GetClipName(index);
    HashString clip = animation->GetClipName(index);

    uint node_idx = animation->FindNode(name);

    // ensure node name doesn´t exists
    uint counter = 0;
    while(node_idx < animation->GetNumNodes())
    {
        char tmp[128];
        snprintf(tmp, 127, "%s_%d", name.C_str(), ++counter);
        name = HashString(tmp);
        node_idx = animation->FindNode(name);
    }

    animation->AddNode(name, clip, 1.0f);
}

void StateViewport::ManageCreate(ComponentAnimation* animation)
{
    if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        auto showLabel = [](const char* label, ImColor color)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
            auto size = ImGui::CalcTextSize(label);

            auto padding = ImGui::GetStyle().FramePadding;
            auto spacing = ImGui::GetStyle().ItemSpacing;

            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

            auto rectMin = ImGui::GetCursorScreenPos() - padding;
            auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
            ImGui::TextUnformatted(label);
        };

        ed::PinId startPinId = 0, endPinId = 0;
        if (ed::QueryNewLink(&startPinId, &endPinId))
        {
            if (startPin && endPin)
            {
                bool startIsInput = uint(startPinId.Get())%2 == 0;
                bool endIsInput   = uint(endPinId.Get())%2 == 0;
                uint startNode    = uint(startPinId.Get())/2-1;
                uint endNode      = uint(endPinId.Get())/2-1;

                if (endPin == startPin)
                {
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (startIsInput == endIsInput)
                {
                    showLabel("x Incompatible Pins. Must be In->Out", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (startNode == endNode)
                {
                    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                }
                else
                {
                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                    if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                    {
                        animation->AddTransition(animation->GetNodeName(startNode), animation->GetNodeName(endNode), HashString(), 300);
                    }
                }
            }
        }

        ed::PinId pinId = 0;
        if (ed::QueryNewNode(&pinId))
        {
            if(pinId != ed::PinId::Invalid)
            {
                showLabel("+ Create Node", ImColor(32, 45, 32, 180));
            }

            if (ed::AcceptNewItem())
            {
                ed::Suspend();
                ImGui::OpenPopup("Create New Node");
                ed::Resume();
            }
        }
    }

    ed::EndCreate();
}

void StateViewport::ShowContextMenus(ComponentAnimation* animation, uint& node_idx, uint& link_idx)
{
    ed::Suspend();

    ed::NodeId contextNodeId = 0;
    ed::PinId contextPinId = 0;
    ed::LinkId contextLinkId = 0;
    if (ed::ShowNodeContextMenu(&contextNodeId))
    {
        ImGui::OpenPopup("Node Context Menu");
    }
    else if (ed::ShowPinContextMenu(&contextPinId))
    {
        ImGui::OpenPopup("Pin Context Menu");
    }
    else if (ed::ShowLinkContextMenu(&contextLinkId))
    {
        ImGui::OpenPopup("Link Context Menu");
    }
    else if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
    }

    ed::Resume();

    node_idx = uint(contextNodeId.Get())-1;
    link_idx = uint(contextLinkId.Get())-1;
}

void StateViewport::ShowCreateNewNodeMenu(ComponentAnimation* animation)
{
    if (ImGui::BeginPopup("Create New Node"))
    {
        ImGui::TextUnformatted("Create Node Menu");
        ImGui::Separator();

        if (ImGui::BeginMenu("New animation"))
        {
            for(uint i=0, count = animation->GetNumClips(); i < count; ++i)
            {
                if(ImGui::MenuItem(animation->GetClipName(i).C_str()))
                {
                    AddAnimationNode(animation, i);
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

void StateViewport::ShowNodeMenu(ComponentAnimation* animation, uint node_idx)
{
    if (ImGui::BeginPopup("Node Context Menu"))
    {
        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();

        ImGui::Text("Name %s", animation->GetNodeName(node_idx).C_str());
        ImGui::Text("Clip %s", animation->GetNodeClip(node_idx).C_str());
        ImGui::Text("Speed: %g", animation->GetNodeSpeed(node_idx));

        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteNode(contextNodeId);
            animation->RemoveNode(node_idx);
        }

        ImGui::EndPopup();
    }
}

void StateViewport::ShowLinkMenu(ComponentAnimation* animation, uint link_idx)
{
    if (ImGui::BeginPopup("Link Context Menu"))
    {
        ImGui::TextUnformatted("Transition Context Menu");
        ImGui::Separator();

        ImGui::Text("Source %s", animation->GetTransitionSource(link_idx).C_str());
        ImGui::Text("Target %s", animation->GetTransitionTarget(link_idx).C_str());
        ImGui::Text("Trigger %s", animation->GetTransitionTrigger(link_idx).C_str());
        ImGui::Text("Blend %d", animation->GetTransitionBlend(link_idx));

        ImGui::Separator();

        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteLink(contextLinkId);
            animation->RemoveTransition(link_idx);
        }

        ImGui::EndPopup();
    }

}
