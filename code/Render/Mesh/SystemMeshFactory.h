#ifndef traktor_render_SystemMeshFactory_H
#define traktor_render_SystemMeshFactory_H

#include "Core/Heap/Ref.h"
#include "Render/Mesh/MeshFactory.h"

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

/*! \brief Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS SystemMeshFactory : public MeshFactory
{
	T_RTTI_CLASS(SystemMeshFactory)

public:
	virtual Mesh* createMesh(
		const std::vector< VertexElement >& vertexElements,
		unsigned int vertexBufferSize,
		IndexType indexType,
		unsigned int indexBufferSize
	);
};

	}
}

#endif	// traktor_render_SystemMeshFactory_H
