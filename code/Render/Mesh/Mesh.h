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

	const std::vector< VertexElement >& getVertexElements() const;

	void setVertexBuffer(VertexBuffer* vertexBuffer);

	Ref< VertexBuffer > getVertexBuffer() const;

	void setIndexBuffer(IndexBuffer* indexBuffer);

	Ref< IndexBuffer > getIndexBuffer() const;

	void setParts(const std::vector< Part >& parts);

	const std::vector< Part >& getParts() const;
	
	void setBoundingBox(const Aabb3& boundingBox);
	
	const Aabb3& getBoundingBox() const;

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
