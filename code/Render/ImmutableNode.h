#ifndef traktor_render_ImmutableNode_H
#define traktor_render_ImmutableNode_H

#include "Render/Node.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Immutable shader graph node.
 * \ingroup Render
 *
 * Immutable shader graph nodes are nodes which
 * doesn't change their appearance, i.e. has a fixed
 * set of input and output pins.
 */
class T_DLLCLASS ImmutableNode : public Node
{
	T_RTTI_CLASS(ImmutableNode)

public:
	struct InputPinDesc
	{
		const wchar_t* name;
		bool optional;
	};

	struct OutputPinDesc
	{
		const wchar_t* name;
	};

	ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins);

	virtual int getInputPinCount() const;

	virtual const InputPin* getInputPin(int index) const;

	virtual int getOutputPinCount() const;

	virtual const OutputPin* getOutputPin(int index) const;

private:
	RefArray< InputPin > m_inputPins;
	RefArray< OutputPin > m_outputPins;

	ImmutableNode(const ImmutableNode&) { T_FATAL_ERROR; }

	ImmutableNode& operator = (const ImmutableNode&) { T_FATAL_ERROR; return *this; }
};

	}
}

#endif	// traktor_render_ImmutableNode_H
