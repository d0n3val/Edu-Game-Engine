#include "Globals.h"
#include "GameObject.h"
#include "QuadTree.h"

#define NE 0
#define SE 1
#define SW 2
#define NW 3

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
	if (IsLeaf() == true && objects.size() < QUADTREE_MAX_ITEMS)
			objects.push_back(go);
	else
	{
		if(IsLeaf() == true)
			CreateChilds();

		objects.push_back(go);
		RedistributeChilds();
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

	float3 bmin(box.minPoint);
	float3 bmax(box.maxPoint);

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
		bool all = true;
		bool intersects[4];
		all &= intersects[0] = childs[0]->box.Intersects(new_box);
		all &= intersects[1] = childs[1]->box.Intersects(new_box);
		all &= intersects[2] = childs[2]->box.Intersects(new_box);
		all &= intersects[3] = childs[3]->box.Intersects(new_box);

		if (all == true)
			++it; // if it hits all childs, better to just keep it here
		else
		{
			it = objects.erase(it);
			if (intersects[0]) childs[0]->Insert(go);
			if (intersects[1]) childs[1]->Insert(go);
			if (intersects[2]) childs[2]->Insert(go);
			if (intersects[3]) childs[3]->Insert(go);
		}
	}
}

void QuadtreeNode::CollectObjects(std::vector<AABB>& objects) const
{
	for (std::list<GameObject*>::const_iterator it = this->objects.begin(); it != this->objects.end(); ++it)
		objects.push_back((*it)->global_bbox.MinimalEnclosingAABB());

	for(int i = 0; i < 4; ++i)
		if(childs[i] != nullptr) childs[i]->CollectObjects(objects);
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

void Quadtree::Clear()
{
	RELEASE(root);
}

void Quadtree::CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const
{
	if(root != nullptr)
		root->CollectBoxes(nodes);
}

void Quadtree::CollectObjects(std::vector<AABB>& objects) const
{
	if(root != nullptr)
		root->CollectObjects(objects);
}
