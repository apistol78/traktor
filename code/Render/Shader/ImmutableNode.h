#ifndef traktor_render_ImmutableNode_H
#define traktor_render_ImmutableNode_H

#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"

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

/*! \brief Immutable shader graph node.
 * \ingroup Render
 *
 * Immutable shader graph nodes are nodes which
 * doesn't change their appearance, i.e. has a fixed
 * set of input and output pins.
 */
class T_DLLCLASS ImmutableNode : public Node
{
	T_RTTI_CLASS;

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

	virtual ~ImmutableNode();

	virtual int getInputPinCount() const T_OVERRIDE T_FINAL;

	virtual const InputPin* getInputPin(int index) const T_OVERRIDE T_FINAL;

	virtual int getOutputPinCount() const T_OVERRIDE T_FINAL;

	virtual const OutputPin* getOutputPin(int index) const T_OVERRIDE T_FINAL;

private:
	std::vector< InputPin* > m_inputPins;
	std::vector< OutputPin* > m_outputPins;

	ImmutableNode& operator = (const ImmutableNode&) { T_FATAL_ERROR; return *this; }
};

	}
}

#endif	// traktor_render_ImmutableNode_H
