#include <cassert>
#include "Render/Dx10/Platform.h"
#include "Render/Dx10/HlslShader.h"

namespace traktor
{
	namespace render
	{

HlslShader::HlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_interpolatorCount(0)
,	m_booleanRegisterCount(0)
,	m_nextTemporaryVariable(0)
,	m_needVPos(false)
{
	pushScope();
	pushOutputStream(BtUniform, new StringOutputStream());
	pushOutputStream(BtInput, new StringOutputStream());
	pushOutputStream(BtOutput, new StringOutputStream());
	pushOutputStream(BtScript, new StringOutputStream());
	pushOutputStream(BtBody, new StringOutputStream());
}

HlslShader::~HlslShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtScript);
	popOutputStream(BtOutput);
	popOutputStream(BtInput);
	popOutputStream(BtUniform);
	popScope();
}

void HlslShader::addInputVariable(const std::wstring& variableName, HlslVariable* variable)
{
	assert (!m_inputVariables[variableName]);
	m_inputVariables[variableName] = variable;
}

HlslVariable* HlslShader::getInputVariable(const std::wstring& variableName)
{
	return m_inputVariables[variableName];
}

HlslVariable* HlslShader::createTemporaryVariable(const OutputPin* outputPin, HlslType type)
{
	StringOutputStream ss;
	ss << L"v" << m_nextTemporaryVariable++;
	return createVariable(outputPin, ss.str(), type);
}

HlslVariable* HlslShader::createVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type)
{
	T_ASSERT (!m_variables.empty());

	Ref< HlslVariable > variable = new HlslVariable(variableName, type);
	m_variables.back().insert(std::make_pair(outputPin, variable));

	return variable;
}

HlslVariable* HlslShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type)
{
	T_ASSERT (!m_variables.empty());

	Ref< HlslVariable > variable = new HlslVariable(variableName, type);
	m_variables.front().insert(std::make_pair(outputPin, variable));

	return variable;
}

void HlslShader::associateVariable(const OutputPin* outputPin, HlslVariable* variable)
{
	m_variables.back().insert(std::make_pair(outputPin, variable));
}

HlslVariable* HlslShader::getVariable(const OutputPin* outputPin)
{
	T_ASSERT (!m_variables.empty());
	for (std::list< scope_t >::reverse_iterator i = m_variables.rbegin(); i != m_variables.rend(); ++i)
	{
		scope_t::iterator j = i->find(outputPin);
		if (j != i->end())
			return j->second;
	}
	return 0;
}

void HlslShader::pushScope()
{
	m_variables.push_back(scope_t());
}

void HlslShader::popScope()
{
	T_ASSERT (!m_variables.empty());
	m_variables.pop_back();
}

int32_t HlslShader::allocateInterpolator()
{
	return m_interpolatorCount++;
}

int32_t HlslShader::allocateBooleanRegister()
{
	return m_booleanRegisterCount++;
}

void HlslShader::allocateVPos()
{
	m_needVPos = true;
}

bool HlslShader::defineScript(const std::wstring& signature)
{
	std::set< std::wstring >::iterator i = m_scripts.find(signature);
	if (i != m_scripts.end())
		return false;

	m_scripts.insert(signature);
	return true;
}

void HlslShader::addSampler(const std::wstring& sampler, const D3D10_SAMPLER_DESC& dsd)
{
	m_samplers.insert(std::make_pair(sampler, dsd));
}

const std::map< std::wstring, D3D10_SAMPLER_DESC >& HlslShader::getSamplers() const
{
	return m_samplers;
}

void HlslShader::addUniform(const std::wstring& uniform)
{
	m_uniforms.insert(uniform);
}

const std::set< std::wstring >& HlslShader::getUniforms() const
{
	return m_uniforms;
}

void HlslShader::pushOutputStream(BlockType blockType, StringOutputStream* outputStream)
{
	m_outputStreams[int(blockType)].push_back(outputStream);
}

void HlslShader::popOutputStream(BlockType blockType)
{
	m_outputStreams[int(blockType)].pop_back();
}

StringOutputStream& HlslShader::getOutputStream(BlockType blockType)
{
	T_ASSERT (!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

std::wstring HlslShader::getGeneratedShader()
{
	StringOutputStream ss;

	ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
	ss << Endl;

	ss << L"uniform float4 _dx10_targetSize;" << Endl;

	std::wstring uniformText = getOutputStream(BtUniform).str();
	if (!uniformText.empty())
	{
		ss << uniformText;
		ss << Endl;
	}

	std::wstring inputDataText = getOutputStream(BtInput).str();
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

	std::wstring outputDataText = getOutputStream(BtOutput).str();
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

	std::wstring scriptText = getOutputStream(BtScript).str();
	if (!scriptText.empty())
		ss << scriptText;

	if (m_shaderType == StVertex)
	{
		ss << L"void main(InputData i";

		if (!outputDataText.empty())
			ss << L", out OutputData o";

		ss << L")" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		if (!outputDataText.empty())
			ss << L"o = (OutputData)0;" << Endl;

		ss << getOutputStream(BtBody).str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}

	if (m_shaderType == StPixel)
	{
		ss << L"void main(";
		
		if (!inputDataText.empty())
			ss << L"InputData i, ";

		if (m_needVPos)
			ss << L"float4 vPos : SV_Position, ";
		
		ss << L"out OutputData o)" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << L"o = (OutputData)0;" << Endl;

		ss << getOutputStream(BtBody).str();

		ss << DecreaseIndent;
		ss << L"}" << Endl;
		ss << Endl;
	}

	return ss.str();
}

	}
}
