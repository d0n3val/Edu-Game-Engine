#include "Globals.h"
#include "ComponentMesh.h"

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container)
{
	type = ComponentTypes::Geometry;
}

// ---------------------------------------------------------
ComponentMesh::~ComponentMesh()
{}

// ---------------------------------------------------------
void ComponentMesh::OnActivate()
{
}

// ---------------------------------------------------------
void ComponentMesh::OnDeActivate()
{
}

// ---------------------------------------------------------
void ComponentMesh::OnStart()
{
}

// ---------------------------------------------------------
void ComponentMesh::OnUpdate()
{
}

// ---------------------------------------------------------
void ComponentMesh::OnFinish()
{
}