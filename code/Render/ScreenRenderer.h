#ifndef traktor_render_ScreenRenderer_H
#define traktor_render_ScreenRenderer_H

#include "Core/Object.h"
#include "Render/Shader.h"

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
class IRenderView;
class RenderTargetSet;
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

	void draw(IRenderView* renderView, Shader* shader);

	void draw(IRenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, Shader* shader);

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	render::Primitives m_primitives;
};

	}
}

#endif	// traktor_render_ScreenRenderer_H
