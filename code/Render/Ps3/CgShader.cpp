#include <cassert>
#include <string>
#include "Render/Ps3/CgShader.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_helperFunctions =
{
	L"float4 columnMajorMul(float4x4 m, float4 v)\n"
	L"{\n"
	L"	return float4(\n"
	L"		dot(m._11_21_31_41, v),\n"
	L"		dot(m._12_22_32_42, v),\n"
	L"		dot(m._13_23_33_43, v),\n"
	L"		dot(m._14_24_34_44, v)\n"
	L"	);\n"
	L"}\n"
};

		}

CgShader::CgShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_interpolatorCount(1)
,	m_nextTemporaryVariable(0)
{
}

CgShader::~CgShader()
{
	for (std::map< OutputPin*, CgVariable* >::iterator i = m_variables.begin(); i != m_variables.end(); ++i)
		delete i->second;
}

void CgShader::addInputPort(const std::wstring& name, DataUsage usage, const CgType& type)
{
	Port port = { usage, type };
	m_inputPorts[name] = port;
}

void CgShader::addOutputPort(const std::wstring& name, DataUsage usage, const CgType& type)
{
	Port port = { usage, type };
	m_outputPorts[name] = port;
}

const CgShader::Port* CgShader::getInputPort(const std::wstring& name) const
{
	std::map< std::wstring, Port >::const_iterator i = m_inputPorts.find(name);
	return (i != m_inputPorts.end()) ? &i->second : 0;
}

const CgShader::Port* CgShader::getOutputPort(const std::wstring& name) const
{
	std::map< std::wstring, Port >::const_iterator i = m_outputPorts.find(name);
	return (i != m_outputPorts.end()) ? &i->second : 0;
}

const std::map< std::wstring, CgShader::Port >& CgShader::getInputPorts() const
{
	return m_inputPorts;
}

const std::map< std::wstring, CgShader::Port >& CgShader::getOutputPorts() const
{
	return m_outputPorts;
}

void CgShader::addInputVariable(const std::wstring& variableName, CgVariable* variable)
{
	assert (!m_inputVariables[variableName]);
	m_inputVariables[variableName] = variable;
}

CgVariable* CgShader::getInputVariable(const std::wstring& variableName)
{
	return m_inputVariables[variableName];
}

CgVariable* CgShader::createTemporaryVariable(OutputPin* outputPin, CgType type)
{
	StringOutputStream ss;
	ss << "v" << m_nextTemporaryVariable++;
	return createVariable(outputPin, ss.str(), type);
}

CgVariable* CgShader::createVariable(OutputPin* outputPin, const std::wstring& variableName, CgType type)
{
	CgVariable* variable = new CgVariable(variableName, type);
	m_variables[outputPin] = variable;
	return variable;
}

CgVariable* CgShader::getVariable(OutputPin* outputPin)
{
	std::map< OutputPin*, CgVariable* >::iterator i = m_variables.find(outputPin);
	return (i != m_variables.end()) ? i->second : 0;
}

int CgShader::allocateInterpolator()
{
	return m_interpolatorCount++;
}

void CgShader::addSampler(const std::wstring& sampler)
{
	m_samplers.insert(sampler);
}

const std::set< std::wstring >& CgShader::getSamplers() const
{
	return m_samplers;
}

void CgShader::addUniform(const std::wstring& uniform)
{
	m_uniforms.insert(uniform);
}

const std::set< std::wstring >& CgShader::getUniforms() const
{
	return m_uniforms;
}

StringOutputStream& CgShader::getFormatter(BlockType blockType)
{
	return m_formatters[int(blockType)];
}

std::wstring CgShader::getGeneratedShader() const
{
	StringOutputStream ss;

	ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
	ss << Endl;

	std::wstring uniformText = m_formatters[BtUniform].str();
	if (!uniformText.empty())
	{
		ss << uniformText;
		ss << Endl;
	}

	std::wstring inputDataText = m_formatters[BtInput].str();
	if (!inputDataText.empty())
	{
		ss << L"struct InputData" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;
		ss << inputDataText;
		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring outputDataText = m_formatters[BtOutput].str();
	if (!outputDataText.empty())
	{
		ss << L"struct OutputData" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;
		ss << outputDataText;
		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	ss << c_helperFunctions;
	ss << Endl;

	if (m_shaderType == StVertex)
	{
		if (!outputDataText.empty())
			ss << L"void main(InputData i, out OutputData o, out float4 _Position_ : TEXCOORD0)" << Endl;
		else
			ss << L"void main(InputData i, out float4 _Position_ : TEXCOORD0)" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		if (!outputDataText.empty())
			ss << L"o = (OutputData)0;" << Endl;
		ss << L"_Position_ = (float4)0;" << Endl;

		ss << m_formatters[BtBody].str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}

	if (m_shaderType == StPixel)
	{
		if (!inputDataText.empty())
			ss << L"void main(InputData i, float4 _Position_ : TEXCOORD0, out OutputData o)" << Endl;
		else
			ss << L"void main(float4 _Position_ : TEXCOORD0, out OutputData o)" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << L"o = (OutputData)0;" << Endl;

		ss << m_formatters[BtBody].str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}

	return ss.str();
}

	}
}
