#include "Render/Context/RenderContext.h"
#include "Spark/CharacterInstance.h"
#include "Spark/CharacterRenderer.h"
#include "Spark/DisplayList.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterRenderer", CharacterRenderer, Object)

bool CharacterRenderer::create(uint32_t frameCount)
{
	// Create render context for each queued frame.
	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(4 * 1024 * 1024);

	// Create global render context.
	m_globalContext = new render::RenderContext(4096);
	return true;
}

void CharacterRenderer::destroy()
{
	m_globalContext = 0;
	m_renderContexts.clear();
}

void CharacterRenderer::build(const CharacterInstance* character, uint32_t frame)
{
	m_renderContexts[frame]->flush();
	character->render(m_renderContexts[frame]);
}

void CharacterRenderer::render(render::IRenderView* renderView, const Matrix44& projection, uint32_t frame)
{
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);
	programParams.setMatrixParameter(L"Spark_Projection", projection);
	programParams.endParameters(m_globalContext);

	m_renderContexts[frame]->render(renderView, render::RpOverlay, &programParams);

	m_globalContext->flush();
}

	}
}
