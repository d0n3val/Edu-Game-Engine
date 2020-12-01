//
//  imgui_color_gradient.cpp
//  imgui extension
//
//  Created by David Gallardo on 11/06/16.


#include "imgui_color_gradient.h"
#include "imgui_internal.h"

static const float GRADIENT_BAR_WIDGET_HEIGHT = 25;
static const float GRADIENT_BAR_EDITOR_HEIGHT = 40;
static const float GRADIENT_MARK_DELETE_DIFFY = 40;

ImGradient::ImGradient()
{
    addMark(0.0f, ImColor(1.0f,1.0f, 1.0f));
    addAlphaMark(0.0f, 1.0f);
    addMark(1.0f, ImColor(1.0f,1.0f,1.0f));
    addAlphaMark(1.0f, 1.0f);
}

ImGradient::~ImGradient()
{
	for (ImGradientMark* mark : m_marks)
	{
		delete mark;
	}
}

ImGradientMark* ImGradient::addMark(float position, ImColor const color)
{
    position = ImClamp(position, 0.0f, 1.0f);
	ImGradientMark* newMark = new ImGradientMark();
    newMark->position = position;
    newMark->color[0] = color.Value.x;
    newMark->color[1] = color.Value.y;
    newMark->color[2] = color.Value.z;
    newMark->alpha    = false;
    
    m_marks.push_back(newMark);
    
    refreshCache();

    return newMark;
}

ImGradientMark* ImGradient::addAlphaMark(float position, float alpha)
{
    position = ImClamp(position, 0.0f, 1.0f);
	ImGradientMark* newMark = new ImGradientMark();
    newMark->position = position;
    newMark->color[0] = alpha;
    newMark->alpha    = true;
    
    m_marks.push_back(newMark);
    
    refreshCache();

    return newMark;
}

void ImGradient::removeMark(ImGradientMark* mark)
{
    m_marks.remove(mark);
    refreshCache();
}

void ImGradient::clearMarks()
{
    m_marks.clear();
    refreshCache();
}

void ImGradient::getColorAt(float position, float* color) const
{
    position = ImClamp(position, 0.0f, 1.0f);    
    int cachePos = int(position * 255);
    cachePos *= 4;
    color[0] = m_cachedValues[cachePos+0];
    color[1] = m_cachedValues[cachePos+1];
    color[2] = m_cachedValues[cachePos+2];

    if (edit_alpha)
    {
        color[3] = m_cachedValues[cachePos + 3];
    }
}

void ImGradient::computeColorAt(float position, float* color) const
{
    position = ImClamp(position, 0.0f, 1.0f);
    
    ImGradientMark* lower = nullptr, *lower_alpha = nullptr;
    ImGradientMark* upper = nullptr, *upper_alpha = nullptr;
    
    for(ImGradientMark* mark : m_marks)
    {
        if(mark->position < position)
        {
            if(!mark->alpha)
            {
                if(!lower || lower->position < mark->position)
                {
                    lower = mark;
                }
            }
            else
            {
                if(!lower_alpha || lower_alpha->position < mark->position)
                {
                    lower_alpha = mark;
                }
            }
        }
        
        if(mark->position >= position)
        {
            if(!mark->alpha)
            {
                if(!upper || upper->position > mark->position)
                {
                    upper = mark;
                }
            }
            else
            {
                if(!upper_alpha || upper_alpha->position > mark->position)
                {
                    upper_alpha = mark;
                }
            }
        }
    }
    
    if(!lower)
    {
        lower = upper;
    }

    if(!upper)
    {
        upper = lower;
    }

    if(!lower_alpha)
    {
        lower_alpha = upper_alpha;
    }

    if(!upper_alpha)
    {
        upper_alpha = lower_alpha;
    }

    if(!lower && !upper)
    {
        color[0] = color[1] = color[2] = 0.0f;
    }
    else if(upper == lower)
    {
        color[0] = upper->color[0];
        color[1] = upper->color[1];
        color[2] = upper->color[2];
    }
    else
    {
        float distance = upper->position - lower->position;
        float delta = (position - lower->position) / distance;
        
        //lerp
        color[0] = ((1.0f - delta) * lower->color[0]) + ((delta) * upper->color[0]);
        color[1] = ((1.0f - delta) * lower->color[1]) + ((delta) * upper->color[1]);
        color[2] = ((1.0f - delta) * lower->color[2]) + ((delta) * upper->color[2]);
    }

    if(!lower_alpha && !upper_alpha)
    {
        color[3] = 1.0f;
    }
    else if(upper_alpha == lower_alpha)
    {
        color[3] = upper_alpha->color[0];
    }
    else
    {
        float distance = upper_alpha->position - lower_alpha->position;
        float delta = (position - lower_alpha->position) / distance;
        
        color[3] = ((1.0f - delta) * lower_alpha->color[0]) + ((delta) * upper_alpha->color[0]);
    }
}

void ImGradient::refreshCache()
{
    m_marks.sort([](const ImGradientMark * a, const ImGradientMark * b) { return a->position < b->position; });
    
    for(int i = 0; i < 256; ++i)
    {
        computeColorAt(i/255.0f, &m_cachedValues[i*4]);
    }
}



namespace ImGui
{
    static void DrawGradientBar(ImGradient* gradient,
                                struct ImVec2 const & bar_pos,
                                float maxWidth,
                                float height)
    {
        ImVec4 colorA = {1,1,1,1};
        ImVec4 colorB = {1,1,1,1};
        float prevX = bar_pos.x;
        float barBottom = bar_pos.y + height;
        ImGradientMark* prevMark = nullptr;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        RenderColorRectWithAlphaCheckerboard(draw_list, ImVec2(bar_pos.x, bar_pos.y),
                                 ImVec2(bar_pos.x + maxWidth, barBottom),
                                 IM_COL32(0, 0, 0, 0), 10.0f, ImVec2(0.0f, 0.0f));
        
        if(gradient->getMarks().size() == 0)
        {
            draw_list->AddRectFilled(ImVec2(bar_pos.x, bar_pos.y),
                                     ImVec2(bar_pos.x + maxWidth, barBottom),
                                     IM_COL32(255, 255, 255, 255));
            
        }
        
        ImU32 colorAU32 = 0;
        ImU32 colorBU32 = 0;
        
        for(auto markIt = gradient->getMarks().begin(); markIt != gradient->getMarks().end(); ++markIt )
        {
            ImGradientMark* mark = *markIt;
            
            float from = prevX;
            float to = prevX = bar_pos.x + mark->position * maxWidth;
            
            gradient->computeColorAt((from-bar_pos.x)/maxWidth, (float*)&colorA);
            gradient->computeColorAt((to-bar_pos.x)/maxWidth, (float*)&colorB);
            
            colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
            colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);
            
            if(mark->position > 0.0)
            {
                
                draw_list->AddRectFilledMultiColor(ImVec2(from, bar_pos.y),
                                                   ImVec2(to, barBottom),
                                                   colorAU32, colorBU32, colorBU32, colorAU32);
            }
            
            prevMark = mark;
        }
        
        if(prevMark && prevMark->position < 1.0)
        {
            
            draw_list->AddRectFilledMultiColor(ImVec2(prevX, bar_pos.y),
                                               ImVec2(bar_pos.x + maxWidth, barBottom),
                                               colorBU32, colorBU32, colorBU32, colorBU32);
        }
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 10.0f));
    }
    
    static void DrawGradientMarks(ImGradient* gradient,
                                  ImGradientMark* & draggingMark,
                                  ImGradientMark* & selectedMark,
                                  struct ImVec2 const & bar_pos,
                                  float maxWidth,
                                  float height)
    {
        ImVec4 color          = {1,1,1,1};
        float barBottom       = bar_pos.y + height;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 colorU32 = 0;
        
        for(auto markIt = gradient->getMarks().begin(); markIt != gradient->getMarks().end(); ++markIt )
        {
            ImGradientMark* mark = *markIt;
            
            float to = bar_pos.x + mark->position * maxWidth;
            gradient->computeColorAt((to-bar_pos.x)/maxWidth, (float*)&color);

            float barY = mark->alpha ? bar_pos.y : barBottom;
            float sign = mark->alpha ? -1.0f : 1.0f;

            if(mark->alpha)
            {
                color.x = color.y = color.z = 0.0f;
            }
            
            colorU32 = ImGui::ColorConvertFloat4ToU32(color);
            
            draw_list->AddTriangleFilled(ImVec2(to, barY -sign*6),
                                         ImVec2(to - 6, barY),
                                         ImVec2(to + 6, barY), IM_COL32(100, 100, 100, 255));
            
            draw_list->AddRectFilled(ImVec2(to - 6, barY),
                                     ImVec2(to + 6, barY+sign*12),
                                     IM_COL32(100, 100, 100, 255), 0.0f, 0);
            
            draw_list->AddRectFilled(ImVec2(to - 5, barY + sign*1),
                                     ImVec2(to + 5, barY + sign*11),
                                     IM_COL32(0, 0, 0, 255), 0.0f, 0);
            
            if(selectedMark == mark)
            {
                draw_list->AddTriangleFilled(ImVec2(to, barY - sign*3),
                                             ImVec2(to - 4, barY + sign*1),
                                             ImVec2(to + 4, barY + sign*1), IM_COL32(0, 255, 0, 255));
                
                draw_list->AddRect(ImVec2(to - 5, barY + sign*1),
                                   ImVec2(to + 5, barY + sign*11),
                                   IM_COL32(0, 255, 0, 255), 0.0f, 0);
            }
            
            draw_list->AddRectFilledMultiColor(ImVec2(to - 3, barY + sign*3),
                                               ImVec2(to + 3, barY + sign*9),
                                               colorU32, colorU32, colorU32, colorU32);
            
            ImGui::SetCursorScreenPos(ImVec2(to - 6, barY+(sign < 0.0f ? -12.0f : 0.0f)));
			ImGui::InvisibleButton("mark", ImVec2(12, 12));
            
            if(ImGui::IsItemHovered())
            {
                if(ImGui::IsMouseClicked(0))
                {
					selectedMark = mark;
					draggingMark = mark;
				}
            }
        }
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 20.0f));
    }
    
    bool GradientButton(ImGradient* gradient)
    {
        if(!gradient) return false;
        
        ImVec2 widget_pos = ImGui::GetCursorScreenPos();
        
        float maxWidth = ImMax(250.0f, ImGui::GetContentRegionAvailWidth() - 100.0f);
        bool clicked = ImGui::InvisibleButton("gradient_bar", ImVec2(maxWidth, GRADIENT_BAR_WIDGET_HEIGHT));

        DrawGradientBar(gradient, widget_pos, maxWidth, GRADIENT_BAR_WIDGET_HEIGHT);
        
        return clicked;
    }
    
    bool GradientEditor(ImGradient* gradient,
                        ImGradientMark* & draggingMark,
                        ImGradientMark* & selectedMark)
    {
        if(!gradient) return false;
        
        bool modified = false;
        
        ImVec2 bar_pos = ImGui::GetCursorScreenPos();
        bar_pos.x += 10;
        bar_pos.y += 10;
        float maxWidth = ImGui::GetContentRegionAvailWidth() - 20;
        float barBottom = bar_pos.y + GRADIENT_BAR_EDITOR_HEIGHT;
        
        DrawGradientBar(gradient, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);
        DrawGradientMarks(gradient, draggingMark, selectedMark, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y -10.0f));
        ImGui::InvisibleButton("gradient_editor_bar", ImVec2(maxWidth, 10.0f));
        
        if(gradient->getEditAlpha() && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
            float pos = (ImGui::GetIO().MousePos.x - bar_pos.x) / maxWidth;
            
            float newMarkCol[4];
            gradient->getColorAt(pos, newMarkCol);
            
            selectedMark = gradient->addAlphaMark(pos, newMarkCol[3]);
        }
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + GRADIENT_BAR_EDITOR_HEIGHT));
        ImGui::InvisibleButton("gradient_editor_bar", ImVec2(maxWidth, 10.0f));
        
        if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
            float pos = (ImGui::GetIO().MousePos.x - bar_pos.x) / maxWidth;
            
            float newMarkCol[4];
            gradient->getColorAt(pos, newMarkCol);
            
            selectedMark = gradient->addMark(pos, ImColor(newMarkCol[0], newMarkCol[1], newMarkCol[2]));
        }
        
        if(!ImGui::IsMouseDown(0) && draggingMark)
        {
            draggingMark = nullptr;
        }
        
        if(ImGui::IsMouseDragging(0) && draggingMark)
        {
            float increment = ImGui::GetIO().MouseDelta.x / maxWidth;
            bool insideZone = (ImGui::GetIO().MousePos.x > bar_pos.x) &&
                              (ImGui::GetIO().MousePos.x < bar_pos.x + maxWidth);
            
            if(increment != 0.0f && insideZone)
            {
                draggingMark->position += increment;
                draggingMark->position = ImClamp(draggingMark->position, 0.0f, 1.0f);
                gradient->refreshCache();
                modified = true;
            }
            
            float diffY = ImGui::GetIO().MousePos.y - barBottom;
            
            if(diffY >= GRADIENT_MARK_DELETE_DIFFY && !draggingMark->alpha)
            {
                gradient->removeMark(draggingMark);
                draggingMark = nullptr;
                selectedMark = nullptr;
                modified = true;
            }

			diffY = ImGui::GetIO().MousePos.y - bar_pos.y;

            if(diffY < -GRADIENT_MARK_DELETE_DIFFY && draggingMark->alpha)
            {
                gradient->removeMark(draggingMark);
                draggingMark = nullptr;
                selectedMark = nullptr;
                modified = true;
            }
        }
        
        if(!selectedMark && gradient->getMarks().size() > 0)
        {
            selectedMark = gradient->getMarks().front();
        }
        
        if(selectedMark)
        {
			if (selectedMark->alpha)
			{
                if(ImGui::SliderFloat("alpha", selectedMark->color, 0.0f, 1.0f))
                {
                    modified = true;
                    gradient->refreshCache();
                }
			}
			else
			{
                
				if((gradient->getEditAlpha() && ImGui::ColorEdit4("color", selectedMark->color)) || (!gradient->getEditAlpha() && ImGui::ColorEdit3("color", selectedMark->color)) )
				{
					modified = true;
					gradient->refreshCache();
				}
			}
        }
        
        return modified;
    }
};

