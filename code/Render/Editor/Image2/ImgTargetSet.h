#pragma once

#include "Render/Editor/ImmutableNode.h"
#include "Render/Frame/RenderGraph.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS ImgTargetSet : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgTargetSet();

	void setTargetSetDesc(const RenderGraphTargetSetDesc& targetSetDesc);

	const RenderGraphTargetSetDesc& getTargetSetDesc() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}   
}