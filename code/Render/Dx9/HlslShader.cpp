#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Render/Dx9/HlslShader.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_registerInternalTargetSize = 0;

		}

HlslShader::HlslShader(ShaderType shaderType)
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

HlslShader::~HlslShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtScript);
	popOutputStream(BtOutput);
	popOutputStream(BtInput);
	popOutputStream(BtUniform);
	popScope();
}

bool HlslShader::haveInput(const std::wstring& inputName) const
{
	return m_inputs.find(inputName) != m_inputs.end();
}

void HlslShader::addInput(const std::wstring& inputName)
{
	m_inputs.insert(inputName);
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

HlslVariable* HlslShader::getVariable(const OutputPin* outputPin) const
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

void HlslShader::pushScope()
{
	m_variables.push_back(scope_t());
}

void HlslShader::popScope()
{
	T_ASSERT (!m_variables.empty());
	m_variables.pop_back();
}

bool HlslShader::defineTexture(const std::wstring& textureName)
{
	m_textures.insert(textureName);
	return true;
}

bool HlslShader::defineSampler(uint32_t samplerHash, const std::wstring& textureName, int32_t& outStage)
{
	std::map< uint32_t, std::pair< std::wstring, int32_t > >::const_iterator i = m_samplers.find(samplerHash);
	if (i != m_samplers.end())
	{
		T_ASSERT (i->second.first == textureName);
		outStage = i->second.second;
		return false;
	}
	else
	{
		outStage = m_nextStage++;
		m_samplers[samplerHash] = std::make_pair(textureName, outStage);
		return true;
	}
}

bool HlslShader::defineScript(const std::wstring& signature)
{
	std::set< std::wstring >::iterator i = m_scripts.find(signature);
	if (i != m_scripts.end())
		return false;

	m_scripts.insert(signature);
	return true;
}

const std::set< std::wstring >& HlslShader::getTextures() const
{
	return m_textures;
}

const std::map< uint32_t, std::pair< std::wstring, int32_t > >& HlslShader::getSamplers() const
{
	return m_samplers;
}

uint32_t HlslShader::addUniform(const std::wstring& uniform, HlslType type, uint32_t count)
{
	const int32_t c_elementCounts[] = { 0, 0, 1, 1, 1, 1, 4, 0 };
	int32_t elementCount = c_elementCounts[int(type)] * count;
	int32_t index = 0;

	// Allocate register with float uniforms.
	if (elementCount > 0)
	{
		// Ensure index are within limits.
		//int32_t fromIndex = 0;
		int32_t toIndex = (m_shaderType == StVertex ? 256 : 224) - elementCount;

		/*
		// Use hash of parameter name to get at least some locality.
		Adler32 cs;
		cs.begin();
		cs.feed(uniform.c_str(), uniform.length() * sizeof(wchar_t));
		cs.end();
		index = (int32_t)cs.get();
		index = fromIndex + index % (toIndex - fromIndex + 1);
		*/

		// Ensure index isn't colliding.
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
			//if (++index >= toIndex)
			//	fromIndex = 0;

			++index;
		}

		for (int32_t i = 0; i < elementCount; ++i)
			m_uniformAllocated[index + i] = true;
	}

	m_uniforms.insert(uniform);
	return index;
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

std::wstring HlslShader::getGeneratedShader(bool needVPos)
{
	StringOutputStream ss;

	ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
	ss << Endl;

	ss << L"uniform float2 __private__targetSize : register(c" << c_registerInternalTargetSize << L");" << Endl;
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
#if defined(_XBOX)
		ss << L"[removeUnusedInputs]" << Endl;
#endif
		ss << L"void main(";
		
		if (!inputDataText.empty())
			ss << L"InputData i, ";

		if (needVPos)
			ss << L"float2 vPos : VPOS, ";
		
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
