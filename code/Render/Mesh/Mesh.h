/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Mesh_H
#define traktor_render_Mesh_H

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Render/VertexElement.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class VertexBuffer;
class IndexBuffer;

/*! \brief Render mesh.
 * \ingroup Render
 */
class T_DLLCLASS Mesh : public Object
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		std::wstring name;
		Primitives primitives;
	};

	void setVertexElements(const std::vector< VertexElement >& vertexDeclaration);

	void setVertexBuffer(VertexBuffer* vertexBuffer);

	void setIndexBuffer(IndexBuffer* indexBuffer);

	void setParts(const std::vector< Part >& parts);

	void setBoundingBox(const Aabb3& boundingBox);

	const std::vector< VertexElement >& getVertexElements() const { return m_vertexElements; }

	VertexBuffer* getVertexBuffer() const { return m_vertexBuffer; }

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

	const std::vector< Part >& getParts() const { return m_parts; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	std::vector< VertexElement > m_vertexElements;
	Ref< VertexBuffer > m_vertexBuffer;
	Ref< IndexBuffer > m_indexBuffer;
	std::vector< Part > m_parts;
	Aabb3 m_boundingBox;
};

	}
}

#endif	// traktor_render_Mesh_H
