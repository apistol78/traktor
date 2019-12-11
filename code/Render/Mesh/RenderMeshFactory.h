#pragma once

#include "Render/Mesh/MeshFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

/*! Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS RenderMeshFactory : public MeshFactory
{
	T_RTTI_CLASS;

public:
	RenderMeshFactory(IRenderSystem* renderSystem);

	virtual Ref< Mesh > createMesh(
		const AlignedVector< VertexElement >& vertexElements,
		uint32_t vertexBufferSize,
		IndexType indexType,
		uint32_t indexBufferSize
	) override final;

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

