#pragma once

#include "Core/Object.h"
#include "Render/Frame/RenderGraph.h"

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

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageTargetSet : public Object
{
	T_RTTI_CLASS;

public:
    ImageTargetSet(const RenderGraphTargetSetDesc& targetSetDesc);

    const RenderGraphTargetSetDesc& getTargetSetDesc() const;

private:
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}
}