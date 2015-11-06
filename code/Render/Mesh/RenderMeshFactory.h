#ifndef traktor_render_RenderMeshFactory_H
#define traktor_render_RenderMeshFactory_H

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
	T_RTTI_CLASS;

public:
	RenderMeshFactory(IRenderSystem* renderSystem);

	virtual Ref< Mesh > createMesh(
		const std::vector< VertexElement >& vertexElements,
		unsigned int vertexBufferSize,
		IndexType indexType,
		unsigned int indexBufferSize
	) T_OVERRIDE T_FINAL;

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_RenderMeshFactory_H
