#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Editor/Node.h"
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

class T_DLLCLASS ImgTargetSet : public Node
{
	T_RTTI_CLASS;

public:
	ImgTargetSet();

	virtual ~ImgTargetSet();

	void setTargetSetDesc(const RenderGraphTargetSetDesc& targetSetDesc);

	const RenderGraphTargetSetDesc& getTargetSetDesc() const;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< InputPin* > m_inputPins;
	AlignedVector< OutputPin* > m_outputPins;
	RenderGraphTargetSetDesc m_targetSetDesc;

	void refresh();
};

	}   
}