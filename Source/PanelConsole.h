#ifndef __PANELCONSOLE_H__
#define __PANELCONSOLE_H__

#include "Panel.h"
#include "Imgui/imgui.h"

class PanelConsole : public Panel
{
public:
	PanelConsole();
	virtual ~PanelConsole();

	void Draw() override;

	void Clear();
	void AddLog(const char* entry);

private:

    ImGuiTextBuffer Buf;
    bool ScrollToBottom;
};

#endif  // __PANELCONSOLE_H__