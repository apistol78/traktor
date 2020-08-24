#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Render/Types.h"
#include "Render/Image2/IImageStep.h"

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

class ImagePassOp;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePass : public IImageStep
{
	T_RTTI_CLASS;

public:
	virtual void addPasses(const ImageGraph* graph, const ImageGraphContext& context, const targetSetVector_t& targetSetIds, RenderGraph& renderGraph) const override final;

private:
	friend class ImagePassData;

	std::wstring m_name;
	int32_t m_outputTargetSet;
	Clear m_clear;
	RefArray< const ImagePassOp > m_ops;
};

	}
}