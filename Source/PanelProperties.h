#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Panel.h"

class PanelProperties : public Panel
{
public:
	PanelProperties();
	virtual ~PanelProperties();

	void Draw() override;

private:

};

#endif// __PANELPROPERTIES_H__