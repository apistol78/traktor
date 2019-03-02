#pragma once

#include <map>
#include <string>
#include <vector>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Render/Editor/Shader/Node.h"

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

class ShaderGraph;

/*! \brief External shader node.
 * \ingroup Render
 *
 * External shader node is a special kind of node
 * which reference another shader graph, a fragment.
 * When the shader is created the external fragment is
 * loaded and merged into the final shader.
 */
class T_DLLCLASS External : public Node
{
	T_RTTI_CLASS;

public:
	External();

	External(const Guid& fragmentGuid, ShaderGraph* fragmentGraph);

	void setFragmentGuid(const Guid& fragmentGuid);

	const Guid& getFragmentGuid() const;

	void setValue(const std::wstring& name, float value);

	float getValue(const std::wstring& name, float defaultValue) const;

	void removeValue(const std::wstring& name);

	const InputPin* createInputPin(const Guid& id, const std::wstring& name, bool optional);

	const OutputPin* createOutputPin(const Guid& id, const std::wstring& name);

	void removeInputPin(const InputPin* inputPin);

	void removeOutputPin(const OutputPin* outputPin);

	virtual std::wstring getInformation() const override final;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

	std::vector< InputPin* >& getInputPins() { return m_inputPins; }

	std::vector< OutputPin* >& getOutputPins() { return m_outputPins; }

	const std::map< std::wstring, float >& getValues() const { return m_values; }

private:
	Guid m_fragmentGuid;
	std::vector< InputPin* > m_inputPins;
	std::vector< OutputPin* > m_outputPins;
	std::map< std::wstring, float > m_values;
};

	}
}

