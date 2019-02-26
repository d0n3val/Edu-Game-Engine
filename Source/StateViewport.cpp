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
        ed::Begin("State machine editor", ImVec2(0.0, 0.0f));

        DrawNodes();
        ShowContextMenus(animation);

        ed::End();
        ed::SetCurrentEditor(nullptr);
    }
}

void StateViewport::TestDraw()
{
    ed::SetCurrentEditor(context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    //ed::PushStyleColor(ed::StyleColor_NodeBg,        ImColor(128, 128, 128, 200));
    //ed::PushStyleColor(ed::StyleColor_NodeBorder,    ImColor( 32,  32,  32, 200));
    ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor( 60, 180, 255, 150));
    ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150));

    //ed::PushStyleVar(ed::StyleVar_NodePadding,  ImVec4(0, 0, 0, 0));
    ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
    ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
    ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
    ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
    ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);

    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
    ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
    ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("In");
    ed::EndPin();
    ed::PopStyleVar(3);

    ImGui::SameLine();
	ed::PinId pin_a_out = uniqueId++;
    ed::BeginPin(pin_a_out, ed::PinKind::Output);
    ImGui::Text("Out");
    ed::EndPin();
    ed::EndNode();
    ed::PopStyleColor(2);
    ed::PopStyleVar(5);

    ed::BeginNode(uniqueId++);
    ImGui::Text("Node B");
	ed::PinId pin_b_in = uniqueId++;
    ed::BeginPin(pin_b_in, ed::PinKind::Input);
    ImGui::Text("In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out");
    ed::EndPin();
	ed::Link(uniqueId++, pin_a_out, pin_b_in);
    ed::EndNode();

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
#if 0
            auto startPin = FindPin(startPinId);
            auto endPin   = FindPin(endPinId);

            newLinkPin = startPin ? startPin : endPin;

            if (startPin->Kind == PinKind::Input)
            {
                std::swap(startPin, endPin);
                std::swap(startPinId, endPinId);
            }

            if (startPin && endPin)
            {
                if (endPin == startPin)
                {
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (endPin->Kind == startPin->Kind)
                {
                    showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                //else if (endPin->Node == startPin->Node)
                //{
                //    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                //    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                //}
                else if (endPin->Type != startPin->Type)
                {
                    showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else
                {
                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                    if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                    {
                        // \todo: s_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                        // \todo: s_Links.back().Color = GetIconColor(startPin->Type);
                    }
                }
            }
#endif 

            if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
            {
				int i = 0;
				++i;
            }
        }

        ed::PinId pinId = 0;
        if (ed::QueryNewNode(&pinId))
        {
           // newLinkPin = FindPin(pinId);
            //if (newLinkPin)
            if(pinId != ed::PinId::Invalid)
                showLabel("+ Create Node", ImColor(32, 45, 32, 180));
			else
			{
				int i = 0;
				++i;
			}

            if (ed::AcceptNewItem())
            {
                /*
                createNewNode  = true;
                newNodeLinkPin = FindPin(pinId);
                newLinkPin = nullptr;
                ed::Suspend();
                ImGui::OpenPopup("Create New Node");
                ed::Resume();
            */
            }
        }
    }

    ed::EndCreate();

    ed::End();
    ed::SetCurrentEditor(nullptr);
}


void StateViewport::DrawNodes()
{
}

void StateViewport::ShowContextMenus(ComponentAnimation* animation)
{
    ed::Suspend();

    if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
    }

    ShowCreateNewNode(animation);

    ed::Resume();
}

void StateViewport::ShowCreateNewNode(ComponentAnimation* animation)
{
    if (ImGui::BeginPopup("Create New Node"))
    {
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

void StateViewport::AddAnimationNode(ComponentAnimation* animation, uint index)
{
}

