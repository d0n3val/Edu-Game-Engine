#ifndef __PANELRESOURCES_H__
#define __PANELRESOURCES_H__

#include "Panel.h"
#include "Resource.h"

class PanelResources : public Panel
{
public:
	PanelResources();
	virtual ~PanelResources();

	void Draw() override;

	UID DrawResourceType(Resource::Type type);

private:

};

#endif  // __PANELRESOURCES_H__