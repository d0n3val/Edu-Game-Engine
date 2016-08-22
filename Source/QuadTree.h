// ----------------------------------------------------
// Quadtree implementation --
// ----------------------------------------------------

#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "Globals.h"
#include "Math.h"
#include "GameObject.h"
#include <vector>

class GameObject;

#define QUADTREE_MAX_ITEMS 2

// Helper function to check if one rectangle complately contains another
//bool Contains(const SDL_Rect& a, const SDL_Rect& b);
//bool Intersects(const SDL_Rect& a, const SDL_Rect& b);

#define NE 0
#define SE 1
#define SW 2
#define NW 3

/*
		----------- MaxPoint
		| NW | NE |
		|---------|
		| SW | SE |
		-----------
MinPoint
*/

// Tree node -------------------------------------------------------
class QuadtreeNode
{

public:
	ALIGN_CLASS_TO_16

	AABB					 box;
	std::vector<GameObject*> objects;
	QuadtreeNode*			 parent;
	QuadtreeNode*			 childs[4];

public:

	QuadtreeNode(const AABB& box) : box(box), objects(QUADTREE_MAX_ITEMS)
	{
		parent = childs[NE] = childs[SE] = childs[SW] = childs[NW] = nullptr;
	}

	~QuadtreeNode()
	{
		for(int i = 0; i < 4; ++i)
			if(childs[i] != nullptr) RELEASE(childs[i]);
	}
	 
	void Insert(GameObject* go)
	{
		// TODO: Insertar un nou Collider al quadtree
		// En principi cada node por enmagatzemar QUADTREE_MAX_ITEMS nodes (encara que podrien ser més)
		// Si es detecten més, el node s'ha de tallar en quatre
		// Si es talla, a de redistribuir tots els seus colliders pels nous nodes (childs) sempre que pugui
		// Nota: un Collider pot estar a més de un node del quadtree
		// Nota: si un Collider intersecciona als quatre childs, deixar-lo al pare

		if (objects.size() < QUADTREE_MAX_ITEMS)
			objects.push_back(go);
		else
		{
			CreateChilds();
			AABB new_box(go->global_bbox.MinimalEnclosingAABB());

			// Now distribute this new gameobject onto the childs
			bool all = true;
			bool intersects[4];
			all &= intersects[0] = childs[0]->box.Intersects(new_box);
			all &= intersects[1] = childs[1]->box.Intersects(new_box);
			all &= intersects[2] = childs[2]->box.Intersects(new_box);
			all &= intersects[3] = childs[3]->box.Intersects(new_box);

			if (all == true)
				objects.push_back(go); // if it hits all childs, better to just keep it here
			else
			{
				if (intersects[0]) childs[0]->Insert(go);
				if (intersects[1]) childs[1]->Insert(go);
				if (intersects[2]) childs[2]->Insert(go);
				if (intersects[3]) childs[3]->Insert(go);
			}
		}
	}

	void CreateChilds()
	{
		// We need to subdivide this node ...
		float3 size(box.Size());
		float3 new_size(size.x*0.5f, size.y, size.z*0.5f); // Octree would subdivide y too

		float3 bmin(box.minPoint);
		float3 bmax(box.maxPoint);

		float3 center;
		AABB new_box;

		// NorthEast
		center.Set(bmin.x + size.x * 0.75f, box.minPoint.y, bmin.z + size.z * 0.75f);
		new_box.SetFromCenterAndSize(center, new_size);
		childs[NE] = new QuadtreeNode(new_box);

		// SouthEast
		center.Set(bmin.x + size.x * 0.75f, box.minPoint.y, bmin.z + size.z * 0.25f);
		new_box.SetFromCenterAndSize(center, new_size);
		childs[SE] = new QuadtreeNode(new_box);

		// SouthWest
		center.Set(bmin.x + size.x * 0.25f, box.minPoint.y, bmin.z + size.z * 0.25f);
		new_box.SetFromCenterAndSize(center, new_size);
		childs[SW] = new QuadtreeNode(new_box);

		// NorthWest
		center.Set(bmin.x + size.x * 0.25f, box.minPoint.y, bmin.z + size.z * 0.75f);
		new_box.SetFromCenterAndSize(center, new_size);
		childs[SW] = new QuadtreeNode(new_box);
	}

	/*
	int CollectCandidates(p2DynArray<Collider*>& nodes, const SDL_Rect& r) const
	{
		// TODO:
		// Omplir el array "nodes" amb tots els colliders candidats
		// de fer intersecció amb el rectangle r
		// retornar el número de intersección calculades en el procés
		// Nota: és una funció recursiva
		return 0;
	}*/

	void CollectBoxes(std::vector<QuadtreeNode*>& nodes) 
	{
		nodes.push_back(this);

		for(int i = 0; i < 4; ++i)
			if(childs[i] != nullptr) childs[i]->CollectBoxes(nodes);
	}

};

// Tree class -------------------------------------------------------
class Quadtree
{
public:
	Quadtree()
	{}

	// Destructor
	virtual ~Quadtree()
	{
		Clear();
	}

	void SetBoundaries(const AABB& box)
	{
		RELEASE(root);
		root = new QuadtreeNode(box);
	}

	void Insert(GameObject* go)
	{
		if(root != nullptr)
		{
			if(go->global_bbox.MinimalEnclosingAABB().Intersects(root->box))
				root->Insert(go);
		}
	}

	void Clear()
	{
		RELEASE(root);
	}

	/*
	int CollectCandidates(std::vector<GameObject*>& nodes, const SDL_Rect& r) const
	{
		int tests = 1;
		if(root != NULL && Intersects(root->rect, r))
			tests = root->CollectCandidates(nodes, r);
		return tests;
	}
	*/

	void CollectBoxes(std::vector<QuadtreeNode*>& nodes) const
	{
		if(root != nullptr)
			root->CollectBoxes(nodes);
	}
	

public:
	QuadtreeNode*	root = nullptr;
};

#endif // __QuadTree_H__
