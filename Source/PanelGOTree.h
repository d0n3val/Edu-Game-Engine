#ifndef __PANELGOTREE_H__
#define __PANELGOTREE_H__

// Editor Panel to show the full tree of game objects of the scene
#include "Panel.h"

class GameObject;

class PanelGOTree : public Panel
{
public:
	PanelGOTree();
	virtual ~PanelGOTree();

	void Draw() override;
	void RecursiveDraw(const GameObject* go);

private:

	uint node = 0;
	char name[80];
};

#endif// __PANELGOTREE_H__