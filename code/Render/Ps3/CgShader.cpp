#include "Render/Ps3/CgShader.h"
#include "Core/Misc/Adler32.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

CgShader::CgShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_uniformAllocated(256, false)
,	m_nextTemporaryVariable(0)
{
	pushScope();
	pushOutputStream(BtUniform, new StringOutputStream());
	pushOutputStream(BtInput, new StringOutputStream());
	pushOutputStream(BtOutput, new StringOutputStream());
	pushOutputStream(BtBody, new StringOutputStream());
}

CgShader::~CgShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtOutput);
	popOutputStream(BtInput);
	popOutputStream(BtUniform);
	popScope();
}

bool CgShader::haveInput(const std::wstring& inputName) const
{
	return m_inputs.find(inputName) != m_inputs.end();
}

void CgShader::addInput(const std::wstring& inputName)
{
	m_inputs.insert(inputName);
}

CgVariable* CgShader::createTemporaryVariable(const OutputPin* outputPin, CgType type)
{
	StringOutputStream ss;
	ss << L"v" << m_nextTemporaryVariable++;
	return createVariable(outputPin, ss.str(), type);
}

CgVariable* CgShader::createVariable(const OutputPin* outputPin, const std::wstring& variableName, CgType type)
{
	T_ASSERT (!m_variables.empty());

	CgVariable* variable = new CgVariable(variableName, type);
	m_variables.back().insert(std::make_pair(outputPin, variable));

	return variable;
}

CgVariable* CgShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, CgType type)
{
	T_ASSERT (!m_variables.empty());

	CgVariable* variable = new CgVariable(variableName, type);
	m_variables.front().insert(std::make_pair(outputPin, variable));

	return variable;
}

CgVariable* CgShader::getVariable(const OutputPin* outputPin) const
{
	T_ASSERT (!m_variables.empty());

	for (std::list< scope_t >::const_reverse_iterator i = m_variables.rbegin(); i != m_variables.rend(); ++i)
	{
		scope_t::const_iterator j = i->find(outputPin);
		if (j != i->end())
			return j->second;
	}

	return 0;
}

void CgShader::pushScope()
{
	m_variables.push_back(scope_t());
}

void CgShader::popScope()
{
	T_ASSERT (!m_variables.empty());
	for (scope_t::iterator i = m_variables.back().begin(); i != m_variables.back().end(); ++i)
		delete i->second;
	m_variables.pop_back();
}

void CgShader::addSampler(const std::wstring& sampler)
{
	m_samplers.insert(sampler);
}

const std::set< std::wstring >& CgShader::getSamplers() const
{
	return m_samplers;
}

uint32_t CgShader::addUniform(const std::wstring& uniform, CgType type, uint32_t count)
{
	const int32_t c_elementCounts[] = { 0, 0, 1, 1, 1, 1, 4 };
	int32_t elementCount = c_elementCounts[int(type)] * count;
	int32_t index = 0;

	if (elementCount > 0)
	{
		int32_t fromIndex, toIndex;

		if (count <= 1)	// Non-indexed uniform are placed at the lower half.
		{
			fromIndex = 0;
			toIndex = 128 - elementCount;
		}
		else	// Upper half.
		{
			fromIndex = 128;
			toIndex = (m_shaderType == StVertex ? 256 : 224) - elementCount;
			if (toIndex < fromIndex)
			{
				log::error << L"Indexed array out-of-range; too many elements " << elementCount << Endl;
				T_FATAL_ERROR;
			}
		}

		Adler32 cs;
		cs.begin();
		cs.feed(uniform.c_str(), uniform.length() * sizeof(wchar_t));
		cs.end();

		index = fromIndex + cs.get() % (toIndex - fromIndex + 1);

		for (;;)
		{
			bool occupied = false;
			for (int32_t i = 0; i < elementCount; ++i)
			{
				if (m_uniformAllocated[index + i])
				{
					occupied = true;
					break;
				}
			}
			if (!occupied)
				break;
			if (++index >= toIndex)
				fromIndex = 0;
		}

		for (int32_t i = 0; i < elementCount; ++i)
			m_uniformAllocated[index + i] = true;
	}

	m_uniforms.insert(uniform);
	return index;
}

const std::set< std::wstring >& CgShader::getUniforms() const
{
	return m_uniforms;
}

void CgShader::pushOutputStream(BlockType blockType, StringOutputStream* outputStream)
{
	m_outputStreams[int(blockType)].push_back(outputStream);
}

void CgShader::popOutputStream(BlockType blockType)
{
	m_outputStreams[int(blockType)].pop_back();
}

StringOutputStream& CgShader::getOutputStream(BlockType blockType)
{
	T_ASSERT (!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

std::wstring CgShader::getGeneratedShader(bool needVPos)
{
	StringOutputStream ss;

	ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
	ss << Endl;

	ss << L"float4 columnMajorMul(float4x4 m, float4 v)" << Endl;
	ss << L"{" << Endl;
	ss << L"	return float4(" << Endl;
	ss << L"		dot(m._11_21_31_41, v)," << Endl;
	ss << L"		dot(m._12_22_32_42, v)," << Endl;
	ss << L"		dot(m._13_23_33_43, v)," << Endl;
	ss << L"		dot(m._14_24_34_44, v)" << Endl;
	ss << L"	);" << Endl;
	ss << L"}" << Endl;
	ss << Endl;

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
#if defined(_XBOX)
		ss << L"[removeUnusedInputs]" << Endl;
#endif
		ss << L"void main(";
		
		if (!inputDataText.empty())
			ss << L"InputData i, ";

		if (needVPos)
			ss << L"float2 vPos : WPOS, ";
		
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
