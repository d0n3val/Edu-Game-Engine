#include "Globals.h"
#include "StateViewport.h"
#include "ComponentAnimation.h"
#include "ResourceStateMachine.h"

#include "mmgr/mmgr.h"

#define DEFAULT_BLEND 300

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
        ResourceStateMachine* state_machine = animation->GetResource();

        if(state_machine != nullptr)
        {
            ed::SetCurrentEditor(context);
            ed::Begin("State Machine Editor", ImVec2(0.0, 0.0f));

            DrawNodes(state_machine);
            DrawTransitions(state_machine);
            ManageCreate(state_machine);

            ShowContextMenus(state_machine);

            ed::Suspend();
            ShowNodeMenu(state_machine);
            ShowLinkMenu(state_machine);
            ShowCreateNewNodeMenu(state_machine);
            ed::Resume();

            ed::End();
            ed::SetCurrentEditor(nullptr);
        }
    }
}

bool StateViewport::GetClip(void* data, int idx, const char** out_text)
{
    ResourceStateMachine* animation = reinterpret_cast<ResourceStateMachine*>(data);
    if(uint(idx) < animation->GetNumClips())
    {
        *out_text = animation->GetClipName(idx).C_str();

        return true;
    }

    return false;
}

bool StateViewport::GetNode(void* data, int idx, const char** out_text)
{
    ResourceStateMachine* animation = reinterpret_cast<ResourceStateMachine*>(data);
    if(uint(idx) < animation->GetNumNodes())
    {
        *out_text = animation->GetNodeName(idx).C_str();

        return true;
    }

    return false;
}

void StateViewport::DrawNodes(ResourceStateMachine* animation)
{
    for(uint i=0, count = animation->GetNumNodes(); i < count; ++i)
    {
        ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor( 60, 180, 255, 150));
        ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150));

        ed::BeginNode(i*3+1);
        ImGui::Indent(1.0);
        ImGui::TextColored(ImVec4(255, 255, 0, 255), animation->GetNodeName(i).C_str());
		const float commentAlpha = 0.5f;

		auto alpha = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

        ImVec2 size = ed::GetNodeSize(i*3+1);
        ImVec2 pos = ed::GetNodePosition(i*3+2);

		ImDrawList* drawList = ed::GetNodeBackgroundDrawList(i * 3 + 1);

		drawList->AddLine(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos()+ImVec2(size.x-16.0f, 0.0),
			IM_COL32(255, 255, 0, 255), 1.0f);


		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(96.0, 8.0));
        ImGui::BulletText("Clip: %s", animation->GetNodeClip(i).C_str());
		ImGui::Dummy(ImVec2(96.0, 8.0));

		drawList->AddLine(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos()+ImVec2(size.x-16.0f, 0.0),
			IM_COL32(255, 255, 255, 255), 1.0f);

		ImGui::Dummy(ImVec2(64.0, 8.0));

        // In Pin
        ed::PushStyleVar(ed::StyleVar_PinArrowSize, 8.0f);
        ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 8.0f);
        ed::PushStyleVar(ed::StyleVar_PinRadius, 10.0f);
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
        ed::BeginPin(i*3+2, ed::PinKind::Input);
        ImGui::Text("In");
        ed::EndPin();
        ed::PopStyleVar(4);

        // Out Pin
        ImGui::SameLine(size.x-40);
        ed::PushStyleVar(ed::StyleVar_PinArrowSize, 0.0f);
        ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 0.0f);
        ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
        ed::BeginPin(i*3+3, ed::PinKind::Output);
        ImGui::Text("Out");

        ed::EndPin();

		ed::EndNode();

        ed::PopStyleVar(3);
        ed::PopStyleColor(2);

    }
}

void StateViewport::DrawTransitions(ResourceStateMachine* animation)
{
    ed::PushStyleVar(ed::StyleVar_LinkStrength, 4.0f);
    uint num_nodes = animation->GetNumNodes();
    for(uint i=0, count = animation->GetNumTransitions(); i < count; ++i)
    {
        uint source = animation->FindNode(animation->GetTransitionSource(i));
        uint target = animation->FindNode(animation->GetTransitionTarget(i));

        if(source < num_nodes && target < num_nodes)
        {
            ed::Link(num_nodes*3+i+1, source*3+3, target*3+2);
        }
    }
    ed::PopStyleVar(1);
}

void StateViewport::AddAnimationNode(ResourceStateMachine* animation, uint index)
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

    animation->AddNode(name, clip);
    animation->Save();
}

void StateViewport::ManageCreate(ResourceStateMachine* animation)
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
            if (startPinId && endPinId)
            {
                bool startIsInput = uint(startPinId.Get()-1)%3 == 1;
                bool endIsInput   = uint(endPinId.Get()-1)%3 == 1;
                uint startNode    = uint(startPinId.Get()-1)/3;
                uint endNode      = uint(endPinId.Get()-1)/3;

                if (endPinId == startPinId)
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
                        if(startIsInput)
                        {
                            animation->AddTransition(animation->GetNodeName(endNode), animation->GetNodeName(startNode), HashString(), DEFAULT_BLEND);
                        }
                        else
                        {
                            animation->AddTransition(animation->GetNodeName(startNode), animation->GetNodeName(endNode), HashString(), DEFAULT_BLEND);
                        }

                        animation->Save();
                    }
                }
            }
        }

        if (ed::QueryNewNode(&new_node_pin))
        {
            bool pinIsInput = uint(new_node_pin.Get()-1)%3 == 1;

            if(!pinIsInput)
            {
                if(new_node_pin != ed::PinId::Invalid)
                {
                    showLabel("+ Create Node", ImColor(32, 45, 32, 180));
                }

                if (ed::AcceptNewItem())
                {
                    ed::Suspend();
                    new_node_pos = ImGui::GetMousePos();
                    ImGui::OpenPopup("Create New Node");
                    ed::Resume();
                }
            }
        }
    }

    ed::EndCreate();
}

void StateViewport::ShowContextMenus(ResourceStateMachine* animation)
{
    ed::Suspend();

    ed::NodeId contextNodeId = 0;
    ed::PinId contextPinId = 0;
    ed::LinkId contextLinkId = 0;
    if (ed::ShowNodeContextMenu(&contextNodeId))
    {
        context_node = uint(contextNodeId.Get()-1)/3;
        ImGui::OpenPopup("Node Context Menu");
    }
    else if (ed::ShowPinContextMenu(&contextPinId))
    {
        ImGui::OpenPopup("Pin Context Menu");
    }
    else if (ed::ShowLinkContextMenu(&contextLinkId))
    {
        context_link = uint(contextLinkId.Get())-animation->GetNumNodes()*3-1;
        ImGui::OpenPopup("Link Context Menu");
    }
    else if (ed::ShowBackgroundContextMenu())
    {
        new_node_pos = ImGui::GetMousePos();
        new_node_pin = 0;
        ImGui::OpenPopup("Create New Node");
    }

    ed::Resume();

}

void StateViewport::ShowCreateNewNodeMenu(ResourceStateMachine* animation)
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
                    uint node_idx = animation->GetNumNodes();
                    ed::SetNodePosition(node_idx*3+1, ed::ScreenToCanvas(new_node_pos));
                    AddAnimationNode(animation, i);

                    if(new_node_pin != ed::PinId::Invalid)
                    {
                        uint out_node = uint(new_node_pin.Get()-1)/3;
                        animation->AddTransition(animation->GetNodeName(out_node), animation->GetNodeName(node_idx), HashString(), DEFAULT_BLEND);
                        animation->Save();
                    }
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

void StateViewport::ShowNodeMenu(ResourceStateMachine* animation)
{
    if (ImGui::BeginPopup("Node Context Menu"))
    {
        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();

        char tmp[128];

        snprintf(tmp, 127, animation->GetNodeName(context_node).C_str());
        if(ImGui::InputText("Name", tmp, 128))
        {
            animation->SetNodeName(context_node, HashString(tmp));
            animation->Save();
        }

        int clip_index = animation->FindClip(animation->GetNodeClip(context_node));
        if(ImGui::Combo("Clip", &clip_index, &StateViewport::GetClip, animation, animation->GetNumClips()))
        {
            animation->SetNodeClip(context_node, animation->GetClipName(clip_index));
            animation->Save();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteNode(ed::NodeId((context_node+1)*3));
            animation->RemoveNode(context_node);
            animation->Save();
        }

        ImGui::EndPopup();
    }
}

void StateViewport::ShowLinkMenu(ResourceStateMachine* animation)
{
    if (ImGui::BeginPopup("Link Context Menu"))
    {
        ImGui::TextUnformatted("Transition Context Menu");
        ImGui::Separator();

        char tmp[128];

        HashString trigger = animation->GetTransitionTrigger(context_link); 
        snprintf(tmp, 127, trigger ? trigger.C_str() : "");
        if(ImGui::InputText("Trigger", tmp, 128))
        {
            animation->SetTransitionTrigger(context_link, HashString(tmp));
            animation->Save();
        }

        uint blend = animation->GetTransitionBlend(context_link);
        if(ImGui::DragInt("Blend", (int*)&blend, 1.0f, 0, 1000))
        {
            animation->SetTransitionBlend(context_node, blend);
            animation->Save();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteLink(ed::LinkId((context_link+1)+2));
            animation->RemoveTransition(context_link);
            animation->Save();
        }

        ImGui::EndPopup();
    }

}
