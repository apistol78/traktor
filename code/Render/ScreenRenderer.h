#pragma once

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
class IRenderSystem;
class IRenderView;
class ProgramParameters;
class RenderContext;
class Shader;
class VertexBuffer;

/*! Screen renderer.
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

	// \name Direct draws.
	// !{

	void draw(IRenderView* renderView, IProgram* program);

	void draw(IRenderView* renderView, Shader* shader);

	// !}

	// \name Context queued draws.
	// !{

	void draw(RenderContext* renderContext, IProgram* program, ProgramParameters* programParams = nullptr);

	void draw(RenderContext* renderContext, Shader* shader, ProgramParameters* programParams = nullptr);

	// !}

private:
	Ref< VertexBuffer > m_vertexBuffer;
	Primitives m_primitives;
};

	}
}

