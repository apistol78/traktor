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

	explicit ImgTargetSet(const std::wstring& targetSetId);

	void setTargetSetId(const std::wstring& targetSetId);

	const std::wstring& getTargetSetId() const;

	void setTargetSetDesc(const RenderGraphTargetSetDesc& targetSetDesc);

	const RenderGraphTargetSetDesc& getTargetSetDesc() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_targetSetId;
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}   
}