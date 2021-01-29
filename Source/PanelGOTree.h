#ifndef __PANELGOTREE_H__
#define __PANELGOTREE_H__

// Editor Panel to show the full tree of game objects of the scene
#include "Panel.h"

class GameObject;
class DirLight;
class AmbientLight;

class PanelGOTree : public Panel
{
public:
	PanelGOTree();
	virtual ~PanelGOTree();

	void Draw() override;

private:

    void DrawLights();
    void DrawSkybox();
	void RecursiveDraw(GameObject* go);
	void CheckHover(GameObject* go);

public:

	uint node = 0;
	char name[80];
	GameObject* drag = nullptr;
	bool open_selected = false;
};

#endif// __PANELGOTREE_H__
