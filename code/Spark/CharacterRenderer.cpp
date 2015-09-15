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
		m_renderContexts[i] = new render::RenderContext(2 * 1024 * 1024);

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

void CharacterRenderer::render(render::IRenderView* renderView, const Vector2& viewOffset, const Vector2& viewSize, uint32_t frame)
{
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);

	Vector4 viewOffsetSize(viewOffset.x, viewOffset.y, viewSize.x, viewSize.y);
	programParams.setVectorParameter(L"Spark_ViewOffsetSize", viewOffsetSize);

	programParams.endParameters(m_globalContext);

	m_renderContexts[frame]->render(renderView, render::RpOverlay, &programParams);

	m_globalContext->flush();
}

	}
}
