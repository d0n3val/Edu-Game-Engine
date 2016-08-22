// ----------------------------------------------------
// Quadtree implementation --
// ----------------------------------------------------

#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "Math.h"
#include <list>

class GameObject;

#define QUADTREE_MAX_ITEMS 2

// Tree node -------------------------------------------------------
class QuadtreeNode
{

public:
	ALIGN_CLASS_TO_16

public:

	QuadtreeNode(const AABB& box);
	virtual ~QuadtreeNode();

	bool IsLeaf() const;
	 
	void Insert(GameObject* go);
	void CreateChilds();
	void RedistributeChilds();
	void CollectObjects(std::vector<AABB>& objects) const;
	void CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const;

public:
	AABB box;
	std::list<GameObject*> objects;
	QuadtreeNode* parent;
	QuadtreeNode* childs[4];

};

// Tree class -------------------------------------------------------
class Quadtree
{
public:
	Quadtree();
	virtual ~Quadtree();
	void SetBoundaries(const AABB& box);
	void Insert(GameObject* go);
	void Clear();
	void CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const;
	void CollectObjects(std::vector<AABB>& objects) const;

public:
	QuadtreeNode* root = nullptr;
};

#endif // __QUADTREE_H__
