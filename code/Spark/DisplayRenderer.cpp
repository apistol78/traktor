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

	return true;
}

void DisplayRenderer::destroy()
{
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

void DisplayRenderer::render(render::IRenderView* renderView, uint32_t frame)
{
	m_renderContexts[frame]->render(
		renderView, render::RpOverlay,
		0
	);
}

	}
}
