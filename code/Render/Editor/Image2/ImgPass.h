#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Editor/Node.h"

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

class IImgStep;

/*! Image pass definition
 * \ingroup Render
 *
 * A pass contain multiple steps which describe how this pass
 * should be rendered.
 */
class T_DLLCLASS ImgPass : public Node
{
	T_RTTI_CLASS;

public:
	ImgPass();

	virtual ~ImgPass();

	const std::wstring& getName() const;

	const RefArray< IImgStep >& getSteps() const;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	AlignedVector< InputPin* > m_inputPins;
	AlignedVector< OutputPin* > m_outputPins;
	RefArray< IImgStep > m_steps;

	void refresh();
};

	}
}