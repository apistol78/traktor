#ifndef traktor_render_ScreenRenderer_H
#define traktor_render_ScreenRenderer_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Render/Shader.h"

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

class RenderSystem;
class RenderView;
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
	T_RTTI_CLASS(ScreenRenderer)

public:
	ScreenRenderer();

	bool create(RenderSystem* renderSystem);

	void destroy();

	void draw(RenderView* renderView, Shader* shader);

	void draw(RenderView* renderView, RenderTargetSet* renderTargetSet, int renderTarget, Shader* shader);

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	render::Primitives m_primitives;
	render::handle_t m_handleTargetSize;
};

	}
}

#endif	// traktor_render_ScreenRenderer_H
