#ifndef traktor_render_RenderMeshFactory_H
#define traktor_render_RenderMeshFactory_H

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

class IRenderSystem;

/*! \brief Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS RenderMeshFactory : public MeshFactory
{
	T_RTTI_CLASS(RenderMeshFactory)

public:
	RenderMeshFactory(IRenderSystem* renderSystem);

	virtual Mesh* createMesh(
		const std::vector< VertexElement >& vertexElements,
		unsigned int vertexBufferSize,
		IndexType indexType,
		unsigned int indexBufferSize
	);

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_RenderMeshFactory_H
