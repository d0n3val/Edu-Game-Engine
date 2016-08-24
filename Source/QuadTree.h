// ----------------------------------------------------
// Quadtree implementation --
// ----------------------------------------------------

#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "Math.h"
#include <list>
#include <map>

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
	void Erase(GameObject* go);
	void CreateChilds();
	void RedistributeChilds();
	void CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const;
	void CollectObjects(std::vector<GameObject*>& objects) const;
	void CollectObjects(std::map<float, GameObject*>& objects, const float3& origin) const;
	template<typename TYPE>
	void CollectIntersections(std::map<float, GameObject*>& objects, const TYPE& primitive) const;
	template<typename TYPE>
	void CollectIntersections(std::vector<GameObject*>& objects, const TYPE& primitive) const;

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
	void Erase(GameObject* go);
	void Clear();
	void CollectBoxes(std::vector<const QuadtreeNode*>& nodes) const;
	void CollectObjects(std::vector<GameObject*>& objects) const;
	void CollectObjects(std::map<float, GameObject*>& objects, const float3& origin) const;
	template<typename TYPE>
	void CollectIntersections(std::map<float, GameObject*>& objects, const TYPE& primitive) const;
	template<typename TYPE>
	void CollectIntersections(std::vector<GameObject*>& objects, const TYPE& primitive) const;

public:
	QuadtreeNode* root = nullptr;
};

// Intersection methods could use a different number of primitives, so we use a template
template<typename TYPE>
inline void Quadtree::CollectIntersections(std::map<float, GameObject*>& objects, const TYPE & primitive) const
{
	if(root != nullptr)
		root->CollectIntersections(objects, primitive);
}

template<typename TYPE>
inline void Quadtree::CollectIntersections(std::vector<GameObject*>& objects, const TYPE & primitive) const
{
	if(root != nullptr)
		root->CollectIntersections(objects, primitive);
}

template<typename TYPE>
inline void QuadtreeNode::CollectIntersections(std::map<float, GameObject*>& objects, const TYPE & primitive) const
{
	if (primitive.Intersects(box))
	{
		float hit_near, hit_far;
		for (std::list<GameObject*>::const_iterator it = this->objects.begin(); it != this->objects.end(); ++it)
		{
			if (primitive.Intersects((*it)->global_bbox, hit_near, hit_far))
				objects[hit_near] = *it;
		}

		for (int i = 0; i < 4; ++i)
			if (childs[i] != nullptr) childs[i]->CollectIntersections(objects, primitive);
	}
}

template<typename TYPE>
inline void QuadtreeNode::CollectIntersections(std::vector<GameObject*>& objects, const TYPE & primitive) const
{
	if (primitive.Intersects(box))
	{
		for (std::list<GameObject*>::const_iterator it = this->objects.begin(); it != this->objects.end(); ++it)
		{
			if (primitive.Intersects((*it)->global_bbox))
				objects.push_back(*it);
		}

		for (int i = 0; i < 4; ++i)
			if (childs[i] != nullptr) childs[i]->CollectIntersections(objects, primitive);
	}
}

#endif // __QUADTREE_H__