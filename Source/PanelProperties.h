#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Panel.h"

class GameObject;

class PanelProperties : public Panel
{
public:
	PanelProperties();
	virtual ~PanelProperties();

	void Draw() override;

public:
	GameObject* selected = nullptr;

};

#endif// __PANELPROPERTIES_H__