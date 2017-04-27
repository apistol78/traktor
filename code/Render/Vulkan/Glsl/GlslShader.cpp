/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Vulkan/Glsl/GlslShader.h"

namespace traktor
{
	namespace render
	{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_nextTemporaryVariable(0)
,	m_needTargetSize(false)
{
	pushScope();
	pushOutputStream(BtCBufferOnce, new StringOutputStream());
	pushOutputStream(BtCBufferFrame, new StringOutputStream());
	pushOutputStream(BtCBufferDraw, new StringOutputStream());
	pushOutputStream(BtTextures, new StringOutputStream());
	pushOutputStream(BtSamplers, new StringOutputStream());
	pushOutputStream(BtInput, new StringOutputStream());
	pushOutputStream(BtOutput, new StringOutputStream());
	pushOutputStream(BtScript, new StringOutputStream());
	pushOutputStream(BtBody, new StringOutputStream());
}

GlslShader::~GlslShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtScript);
	popOutputStream(BtOutput);
	popOutputStream(BtInput);
	popOutputStream(BtSamplers);
	popOutputStream(BtTextures);
	popOutputStream(BtCBufferDraw);
	popOutputStream(BtCBufferFrame);
	popOutputStream(BtCBufferOnce);
	popScope();
}

void GlslShader::addInputVariable(const std::wstring& variableName, GlslVariable* variable)
{
	T_ASSERT (!m_inputVariables[variableName]);
	m_inputVariables[variableName] = variable;
}

GlslVariable* GlslShader::getInputVariable(const std::wstring& variableName)
{
	return m_inputVariables[variableName];
}

GlslVariable* GlslShader::createTemporaryVariable(const OutputPin* outputPin, GlslType type)
{
	StringOutputStream ss;
	ss << L"v" << m_nextTemporaryVariable++;
	return createVariable(outputPin, ss.str(), type);
}

GlslVariable* GlslShader::createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type)
{
	T_ASSERT (!m_variables.empty());

	Ref< GlslVariable > variable = new GlslVariable(variableName, type);
	m_variables.back().insert(std::make_pair(outputPin, variable));

	return variable;
}

GlslVariable* GlslShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type)
{
	T_ASSERT (!m_variables.empty());

	Ref< GlslVariable > variable = new GlslVariable(variableName, type);
	m_variables.front().insert(std::make_pair(outputPin, variable));

	return variable;
}

void GlslShader::associateVariable(const OutputPin* outputPin, GlslVariable* variable)
{
	m_variables.back().insert(std::make_pair(outputPin, variable));
}

GlslVariable* GlslShader::getVariable(const OutputPin* outputPin)
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

void GlslShader::pushScope()
{
	m_variables.push_back(scope_t());
}

void GlslShader::popScope()
{
	T_ASSERT (!m_variables.empty());
	m_variables.pop_back();
}

void GlslShader::allocateTargetSize()
{
	m_needTargetSize = true;
}

void GlslShader::addUniform(const std::wstring& uniform)
{
	if (std::find(m_uniforms.begin(), m_uniforms.end(), uniform) == m_uniforms.end())
		m_uniforms.push_back(uniform);
}

const std::list< std::wstring >& GlslShader::getUniforms() const
{
	return m_uniforms;
}

bool GlslShader::haveUniform(const std::wstring& uniform) const
{
	return std::find(m_uniforms.begin(), m_uniforms.end(), uniform) != m_uniforms.end();
}

bool GlslShader::defineScript(const std::wstring& signature)
{
	std::set< std::wstring >::iterator i = m_scriptSignatures.find(signature);
	if (i != m_scriptSignatures.end())
		return false;

	m_scriptSignatures.insert(signature);
	return true;
}

void GlslShader::pushOutputStream(BlockType blockType, StringOutputStream* outputStream)
{
	m_outputStreams[int(blockType)].push_back(outputStream);
}

void GlslShader::popOutputStream(BlockType blockType)
{
	m_outputStreams[int(blockType)].pop_back();
}

StringOutputStream& GlslShader::getOutputStream(BlockType blockType)
{
	T_ASSERT (!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

const StringOutputStream& GlslShader::getOutputStream(BlockType blockType) const
{
	T_ASSERT (!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

std::wstring GlslShader::getGeneratedShader() const
{
	StringOutputStream ss;

	ss << L"#version 450" << Endl;
	ss << L"#extension GL_ARB_separate_shader_objects : enable" << Endl;
	ss << L"#extension GL_ARB_shading_language_420pack : enable" << Endl;
	ss << Endl;

	if (m_shaderType == StFragment)
	{
		// Add fragment outputs.
		ss << L"layout (location = 0) out vec4 _gl_FragData_0;" << Endl;
		ss << L"layout (location = 1) out vec4 _gl_FragData_1;" << Endl;
		ss << L"layout (location = 2) out vec4 _gl_FragData_2;" << Endl;
		ss << L"layout (location = 3) out vec4 _gl_FragData_3;" << Endl;
		ss << Endl;
	}

	std::wstring cbufferOnceText = getOutputStream(BtCBufferOnce).str();
	if (!cbufferOnceText.empty())
	{
		if (m_shaderType == StFragment)
			ss << L"layout (std140, binding = 3) uniform cbOnce" << Endl;
		else
			ss << L"layout (std140, binding = 0) uniform cbOnce" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferOnceText;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring cbufferFrameText = getOutputStream(BtCBufferFrame).str();
	if (!cbufferFrameText.empty() || m_needTargetSize)
	{
		if (m_shaderType == StFragment)
			ss << L"layout (std140, binding = 4) uniform cbFrame" << Endl;
		else
			ss << L"layout (std140, binding = 1) uniform cbFrame" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferFrameText;

		if (m_needTargetSize)
			ss << L"vec2 _gl_targetSize;" << Endl;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring cbufferDrawText = getOutputStream(BtCBufferDraw).str();
	if (!cbufferDrawText.empty())
	{
		if (m_shaderType == StFragment)
			ss << L"layout (std140, binding = 5) uniform cbDraw" << Endl;
		else
			ss << L"layout (std140, binding = 2) uniform cbDraw" << Endl;

		ss << L"{" << Endl;
		ss << IncreaseIndent;

		ss << cbufferDrawText;

		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	std::wstring texturesText = getOutputStream(BtTextures).str();
	if (!texturesText.empty())
	{
		ss << texturesText;
		ss << Endl;
	}

	std::wstring samplersText = getOutputStream(BtSamplers).str();
	if (!samplersText.empty())
	{
		ss << samplersText;
		ss << Endl;
	}

	std::wstring inputText = getOutputStream(BtInput).str();
	if (!inputText.empty())
	{
		ss << inputText;
		ss << Endl;
	}

	std::wstring outputText = getOutputStream(BtOutput).str();
	if (!outputText.empty())
	{
		ss << outputText;
		ss << Endl;
	}

	std::wstring scriptText = getOutputStream(BtScript).str();
	if (!scriptText.empty())
	{
		ss << scriptText;
		ss << Endl;
	}

	ss << L"void main()" << Endl;
	ss << L"{" << Endl;
	ss << IncreaseIndent;
	ss << getOutputStream(BtBody).str();
	ss << DecreaseIndent;
	ss << L"}" << Endl;

	return ss.str();
}

	}
}
