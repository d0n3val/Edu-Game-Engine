// This code is in the public domain -- Ignacio Castaño <castano@gmail.com>

#include "BaseMesh.h"
#include "nvcore/Stream.h"
#include "nvmath/TypeSerialization.h"
#include "nvcore/Array.inl"

namespace nv
{
	Stream & operator<< (Stream & s, BaseMesh::Vertex & vertex)
	{
		return s << vertex.id << vertex.pos << vertex.nor << vertex.tex;
	}

	Stream & operator<< (Stream & s, BaseMesh & mesh)
	{
		return s << mesh.m_vertexArray;
	}
}
