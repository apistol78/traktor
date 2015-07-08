#ifndef traktor_render_Script_H
#define traktor_render_Script_H

#include <map>
#include "Render/Shader/Node.h"
#include "Render/Shader/TypedInputPin.h"
#include "Render/Shader/TypedOutputPin.h"

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

/*! \brief Script shader node.
 * \ingroup Render
 *
 * Script shader nodes enabled custom
 * function nodes where functionality
 * is written in pure shader code such as HLSL, GLSL
 * etc.
 */
class T_DLLCLASS Script : public Node
{
	T_RTTI_CLASS;

public:
	virtual ~Script();

	const std::wstring& getName() const;

	void setScript(const std::wstring& script);

	const std::wstring& getScript() const;

	const InputPin* addInputPin(const std::wstring& name, ParameterType type);

	const OutputPin* addOutputPin(const std::wstring& name, ParameterType type);

	void removeInputPin(const std::wstring& name);

	void removeOutputPin(const std::wstring& name);

	ParameterType getInputPinType(int index) const;

	std::wstring getInputPinSamplerId(int index) const;

	ParameterType getOutputPinType(int index) const;

	virtual int getInputPinCount() const;

	virtual const InputPin* getInputPin(int index) const;

	virtual int getOutputPinCount() const;

	virtual const OutputPin* getOutputPin(int index) const;

	const std::map< std::wstring, SamplerState >& getSamplers() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
	std::vector< TypedInputPin* > m_inputPins;
	std::vector< TypedOutputPin* > m_outputPins;
	std::map< std::wstring, SamplerState > m_samplers;
	std::wstring m_script;
};

	}
}

#endif	// traktor_render_Script_H
