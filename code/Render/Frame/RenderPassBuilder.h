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

class T_DLLCLASS RenderPassBuilder : public Object
{
    T_RTTI_CLASS;

public:
    explicit RenderPassBuilder(RenderPass& renderPass);

	void addInput(handle_t targetSetName, int32_t targetColorIndex = -1);

	void setOutput(handle_t targetSetName);

	void setOutput(handle_t targetSetName, const Clear& clear);

private:
    RenderPass& m_renderPass;
};

    }
}