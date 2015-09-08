#ifndef traktor_spark_DisplayRenderer_H
#define traktor_spark_DisplayRenderer_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;
class RenderContext;

	}

	namespace spark
	{

class DisplayList;

/*!
 * Thread 1:
 *		- build 1
 *		- build 2
 *		- ...
 * Thread 2:
 *		- render 1
 *		- render 2
 *		- ...
 */
class T_DLLCLASS DisplayRenderer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(uint32_t frameCount);

	void destroy();

	void build(const DisplayList* displayList, uint32_t frame);

	void render(render::IRenderView* renderView, uint32_t frame);

private:
	RefArray< render::RenderContext > m_renderContexts;
};

	}
}

#endif	// traktor_spark_DisplayRenderer_H
