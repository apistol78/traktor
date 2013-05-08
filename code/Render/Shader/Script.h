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
	const std::wstring& getName() const;

	void setScript(const std::wstring& platform, const std::wstring& script);

	std::wstring getScript(const std::wstring& platform) const;

	virtual int getInputPinCount() const;

	virtual const InputPin* getInputPin(int index) const;

	virtual int getOutputPinCount() const;

	virtual const OutputPin* getOutputPin(int index) const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
	std::vector< TypedInputPin* > m_inputPins;
	std::vector< TypedOutputPin* > m_outputPins;
	std::map< std::wstring, std::wstring > m_scripts;
};

	}
}

#endif	// traktor_render_Script_H
