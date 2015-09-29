#ifndef traktor_render_ScreenRenderer_H
#define traktor_render_ScreenRenderer_H

#include "Core/Object.h"
#include "Render/Types.h"

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

class IProgram;
class IRenderView;
class IRenderSystem;
class RenderTargetSet;
class Shader;
class VertexBuffer;

/*! \brief Screen renderer.
 * \ingroup Render
 *
 * Render full screen quads with given shader.
 * Useful for example when rendering post processing
 * effects and such.
 */
class T_DLLCLASS ScreenRenderer : public Object
{
	T_RTTI_CLASS;

public:
	ScreenRenderer();

	bool create(IRenderSystem* renderSystem);

	void destroy();

	void draw(IRenderView* renderView, IProgram* program);

	void draw(IRenderView* renderView, Shader* shader);

	void draw(IRenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, IProgram* program);

	void draw(IRenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, Shader* shader);

private:
	Ref< VertexBuffer > m_vertexBuffer;
	Primitives m_primitives;
};

	}
}

#endif	// traktor_render_ScreenRenderer_H
