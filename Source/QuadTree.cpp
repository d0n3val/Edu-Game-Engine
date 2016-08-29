#include "Globals.h"
#include "GameObject.h"
#include "QuadTree.h"

#define NE 0
#define SE 1
#define SW 2
#define NW 3

#define QUADTREE_MAX_ITEMS 8
#define QUADTREE_MIN_SIZE 10.0f 

QuadtreeNode::QuadtreeNode(const AABB& box) : box(box)
{
	parent = childs[NE] = childs[SE] = childs[SW] = childs[NW] = nullptr;
}

QuadtreeNode::~QuadtreeNode()
{
	for(int i = 0; i < 4; ++i)
		if(childs[i] != nullptr) RELEASE(childs[i]);
}

bool QuadtreeNode::IsLeaf() const
{
	return childs[0] == nullptr;
}
 
void QuadtreeNode::Insert(GameObject* go)
{
	if (IsLeaf() == true && 
		(objects.size() < QUADTREE_MAX_ITEMS || 
		(box.HalfSize().LengthSq() <= QUADTREE_MIN_SIZE * QUADTREE_MIN_SIZE)))
			objects.push_back(go);
	else
	{
		if(IsLeaf() == true)
			CreateChilds();

		objects.push_back(go);
		RedistributeChilds();
	}
}

void QuadtreeNode::Erase(GameObject * go)
{
	std::list<GameObject*>::iterator it = std::find(objects.begin(), objects.end(), go);
	if (it != objects.end())
		objects.erase(it);

	if (IsLeaf() == false)
	{
		for (int i = 0; i < 4; ++i)
			childs[i]->Erase(go);
	}
}

/*
		----------- MaxPoint
		| NW | NE |
		|---------|
		| SW | SE |
		-----------
MinPoint
*/

void QuadtreeNode::CreateChilds()
{
	// We need to subdivide this node ...
	float3 size(box.Size());
	float3 new_size(size.x*0.5f, size.y, size.z*0.5f); // Octree would subdivide y too

	float3 center(box.CenterPoint());
	float3 new_center(center);
	AABB new_box;

	// NorthEast
	new_center.x = center.x + size.x * 0.25f;
	new_center.z = center.z + size.z * 0.25f;
	new_box.SetFromCenterAndSize(new_center, new_size);
	childs[NE] = new QuadtreeNode(new_box);

	// SouthEast
	new_center.x = center.x + size.x * 0.25f;
	new_center.z = center.z - size.z * 0.25f;
	new_box.SetFromCenterAndSize(new_center, new_size);
	childs[SE] = new QuadtreeNode(new_box);

	// SouthWest
	new_center.x = center.x - size.x * 0.25f;
	new_center.z = center.z - size.z * 0.25f;
	new_box.SetFromCenterAndSize(new_center, new_size);
	childs[SW] = new QuadtreeNode(new_box);

	// NorthWest
	new_center.x = center.x - size.x * 0.25f;
	new_center.z = center.z + size.z * 0.25f;
	new_box.SetFromCenterAndSize(new_center, new_size);
	childs[NW] = new QuadtreeNode(new_box);
}

void QuadtreeNode::RedistributeChilds()
{
	// Now redistribute ALL objects
	for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end();)
	{
		GameObject* go = *it;

		AABB new_box(go->global_bbox.MinimalEnclosingAABB());

		// Now distribute this new gameobject onto the childs
		bool intersects[4];
		for(int i = 0; i < 4; ++i)
			intersects[i] = childs[i]->box.Intersects(new_box);

		if (intersects[0] && intersects[1] && intersects[2] && intersects[3])
			++it; // if it hits all childs, better to just keep it here
		else
		{
			it = objects.erase(it);
			for(int i = 0; i < 4; ++i)
				if (intersects[i]) childs[i]->Insert(go);
		}
	}
}

void QuadtreeNode::CollectObjects(std::vector<GameObject*>& objects) const
{
	for (std::list<GameObject*>::const_iterator it = this->objects.begin(); it != this->objects.end(); ++it)
		objects.push_back(*it);

	for(int i = 0; i < 4; ++i)
		if(childs[i] != nullptr) childs[i]->CollectObjects(objects);
}

void QuadtreeNode::CollectObjects(std::map<float, GameObject*>& objects, const float3& origin) const
{
	for (std::list<GameObject*>::const_iterator it = this->objects.begin(); it != this->objects.end(); ++it)
	{
		float dist = origin.DistanceSq((*it)->GetGlobalPosition());
		objects[dist] = *it;
	}

	for(int i = 0; i < 4; ++i)
		if(childs[i] != nullptr) childs[i]->CollectObjects(objects, origin);
}

void QuadtreeNode::CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const
{
	nodes.push_back(this);

	for(int i = 0; i < 4; ++i)
		if(childs[i] != nullptr) childs[i]->CollectBoxes(nodes);
}

// ---------------------------------------------------------------------

Quadtree::Quadtree()
{}

Quadtree::~Quadtree()
{
	Clear();
}

void Quadtree::SetBoundaries(const AABB& box)
{
	Clear();
	root = new QuadtreeNode(box);
}

void Quadtree::Insert(GameObject* go)
{
	if(root != nullptr)
	{
		if(go->global_bbox.MinimalEnclosingAABB().Intersects(root->box))
			root->Insert(go);
	}
}

void Quadtree::Erase(GameObject * go)
{
	if(root != nullptr)
		root->Erase(go);
}

void Quadtree::Clear()
{
	RELEASE(root);
}

void Quadtree::CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const
{
	if(root != nullptr)
		root->CollectBoxes(nodes);
}

void Quadtree::CollectObjects(std::vector<GameObject*>& objects) const
{
	if(root != nullptr)
		root->CollectObjects(objects);
}

void Quadtree::CollectObjects(std::map<float, GameObject*>& objects, const float3& origin) const
{
	if(root != nullptr)
		root->CollectObjects(objects, origin);
}
