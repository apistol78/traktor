#ifndef traktor_render_CgShader_H
#define traktor_render_CgShader_H

#include <string>
#include <map>
#include <set>
#include "Render/Types.h"
#include "Render/Ps3/CgType.h"
#include "Render/Ps3/CgVariable.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

class CgShader
{
public:
	enum ShaderType
	{
		StVertex,
		StPixel
	};

	enum BlockType
	{
		BtUniform,
		BtInput,
		BtOutput,
		BtBody,
		BtLast
	};

	struct Port
	{
		DataUsage usage;
		CgType type;
	};

	CgShader(ShaderType shaderType);

	virtual ~CgShader();

	void addInputPort(const std::wstring& name, DataUsage usage, const CgType& type);

	void addOutputPort(const std::wstring& name, DataUsage usage, const CgType& type);

	const Port* getInputPort(const std::wstring& name) const;

	const Port* getOutputPort(const std::wstring& name) const;

	const std::map< std::wstring, Port >& getInputPorts() const;

	const std::map< std::wstring, Port >& getOutputPorts() const;

	void addInputVariable(const std::wstring& variableName, CgVariable* variable);

	CgVariable* getInputVariable(const std::wstring& variableName);

	CgVariable* createTemporaryVariable(OutputPin* outputPin, CgType type);

	CgVariable* createVariable(OutputPin* outputPin, const std::wstring& variableName, CgType type);

	CgVariable* getVariable(OutputPin* outputPin);

	int allocateInterpolator();

	void addSampler(const std::wstring& sampler);

	const std::set< std::wstring >& getSamplers() const;

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	StringOutputStream& getFormatter(BlockType blockType);

	std::wstring getGeneratedShader() const;

private:
	ShaderType m_shaderType;
	std::map< std::wstring, Port > m_inputPorts;
	std::map< std::wstring, Port > m_outputPorts;
	std::map< std::wstring, CgVariable* > m_inputVariables;
	std::map< OutputPin*, CgVariable* > m_variables;
	int m_interpolatorCount;
	std::set< std::wstring > m_samplers;
	std::set< std::wstring > m_uniforms;
	int m_nextTemporaryVariable;
	StringOutputStream m_formatters[BtLast];
};

	}
}

#endif	// traktor_render_CgShader_H
