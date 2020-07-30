#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Editor/Node.h"
#include "Render/Frame/RenderGraphTypes.h"

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

	const std::wstring& getTargetSetId() const;

	bool getPersistent() const;

	int32_t getTextureCount() const;

	const std::wstring& getTextureId(int32_t colorIndex) const;

	RenderGraphTargetSetDesc getRenderGraphTargetSetDesc() const;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	struct TargetDesc
	{
		std::wstring textureId;
		render::TextureFormat colorFormat;

		TargetDesc();

		void serialize(ISerializer& s);
	};

	AlignedVector< InputPin* > m_inputPins;
	AlignedVector< OutputPin* > m_outputPins;

	std::wstring m_targetSetId;
	bool m_persistent;
	int32_t m_width;
	int32_t m_height;
	int32_t m_screenWidthDenom;
	int32_t m_screenHeightDenom;
	int32_t m_maxWidth;
	int32_t m_maxHeight;
	bool m_createDepthStencil;
	bool m_ignoreStencil;
	bool m_generateMips;
	AlignedVector< TargetDesc > m_targets;

	void refresh();
};

	}   
}