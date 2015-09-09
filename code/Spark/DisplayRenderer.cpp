#pragma optimize( "", off )

#include "Render/Context/RenderContext.h"
#include "Spark/CharacterInstance.h"
#include "Spark/DisplayList.h"
#include "Spark/DisplayRenderer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.DisplayRenderer", DisplayRenderer, Object)

bool DisplayRenderer::create(uint32_t frameCount)
{
	// Create render context for each queued frame.
	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(2 * 1024 * 1024);

	// Create global render context.
	m_globalContext = new render::RenderContext(4096);
	return true;
}

void DisplayRenderer::destroy()
{
	m_globalContext = 0;
	m_renderContexts.clear();
}

void DisplayRenderer::build(const DisplayList* displayList, uint32_t frame)
{
	m_renderContexts[frame]->flush();

	const SmallMap< int32_t, DisplayList::Layer >& layers = displayList->getLayers();
	for (SmallMap< int32_t, DisplayList::Layer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance != 0);
		i->second.instance->render(m_renderContexts[frame]);
	}
}

void DisplayRenderer::render(render::IRenderView* renderView, const Vector2& viewOffset, const Vector2& viewSize, uint32_t frame)
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
