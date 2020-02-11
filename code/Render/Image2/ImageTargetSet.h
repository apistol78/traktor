#pragma once

#include "Core/Object.h"
#include "Render/Frame/RenderGraphTypes.h"

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
    ImageTargetSet(handle_t targetSetId, handle_t textureIds[RenderGraphTargetSetDesc::MaxColorTargets], const RenderGraphTargetSetDesc& targetSetDesc);

	handle_t getTargetSetId() const;

	handle_t getTextureId(int32_t colorIndex) const;

    const RenderGraphTargetSetDesc& getTargetSetDesc() const;

private:
	handle_t m_targetSetId;
	handle_t m_textureIds[RenderGraphTargetSetDesc::MaxColorTargets];
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}
}