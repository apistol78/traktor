/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps3/Cg/CgShader.h"
#include "Core/Misc/Adler32.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_registerInternalTargetSize = 0;

		}

CgShader::CgShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_uniformAllocated(256, false)
,	m_nextTemporaryVariable(0)
,	m_nextStage(0)
{
	pushScope();
	pushOutputStream(BtUniform, new StringOutputStream());
	pushOutputStream(BtInput, new StringOutputStream());
	pushOutputStream(BtOutput, new StringOutputStream());
	pushOutputStream(BtScript, new StringOutputStream());
	pushOutputStream(BtBody, new StringOutputStream());

	// Ensure internal registers are marked as allocated.
	m_uniformAllocated[c_registerInternalTargetSize] = true;
}

CgShader::~CgShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtScript);
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

	Ref< CgVariable > variable = new CgVariable(variableName, type);
	m_variables.back().insert(std::make_pair(outputPin, variable));

	return variable;
}

CgVariable* CgShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, CgType type)
{
	T_ASSERT (!m_variables.empty());

	Ref< CgVariable > variable = new CgVariable(variableName, type);
	m_variables.front().insert(std::make_pair(outputPin, variable));

	return variable;
}

void CgShader::associateVariable(const OutputPin* outputPin, CgVariable* variable)
{
	m_variables.back().insert(std::make_pair(outputPin, variable));
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
	m_variables.pop_back();
}

bool CgShader::defineScript(const std::wstring& signature)
{
	std::set< std::wstring >::iterator i = m_scriptSignatures.find(signature);
	if (i != m_scriptSignatures.end())
		return false;

	m_scriptSignatures.insert(signature);
	return true;
}

bool CgShader::defineSamplerTexture(const std::wstring& textureName, int32_t& outStage)
{
	std::map< std::wstring, int32_t >::iterator i = m_samplerTextures.find(textureName);
	if (i != m_samplerTextures.end())
	{
		outStage = i->second;
		return false;
	}

	outStage = m_nextStage++;
	m_samplerTextures.insert(std::make_pair(textureName, outStage));

	return true;
}

const std::map< std::wstring, int32_t >& CgShader::getSamplerTextures() const
{
	return m_samplerTextures;
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
				log::error << L"Indexed array out-of-range; Cannot allocate \"" << uniform << L"\", " << elementCount << L" element(s)" << Endl;
				return ~0U;
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
	ss << L"	float4 tmp = v.y * m._21_22_23_24;" << Endl;
	ss << L"	tmp = m._11_12_13_14 * v.x + tmp;" << Endl;
	ss << L"	tmp = m._31_32_33_34 * v.z + tmp;" << Endl;
	ss << L"	return m._41_42_43_44 * v.w + tmp;" << Endl;
	ss << L"}" << Endl;
	ss << Endl;

	ss << L"uniform float2 _cg_targetSize : register(c" << c_registerInternalTargetSize << L");" << Endl;
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
