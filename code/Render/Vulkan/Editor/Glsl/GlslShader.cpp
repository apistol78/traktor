#include "Render/Editor/Shader/OutputPin.h"
#include "Render/Vulkan/Editor/Glsl/GlslLayout.h"
#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"
#include "Render/Vulkan/Editor/Glsl/GlslShader.h"
#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"
#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_nextTemporaryVariable(0)
{
	pushScope();
	//pushOutputStream(BtCBufferOnce, new StringOutputStream());
	//pushOutputStream(BtCBufferFrame, new StringOutputStream());
	//pushOutputStream(BtCBufferDraw, new StringOutputStream());
	//pushOutputStream(BtTextures, new StringOutputStream());
	//pushOutputStream(BtSamplers, new StringOutputStream());
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
	//popOutputStream(BtSamplers);
	//popOutputStream(BtTextures);
	//popOutputStream(BtCBufferDraw);
	//popOutputStream(BtCBufferFrame);
	//popOutputStream(BtCBufferOnce);
	popScope();
}

void GlslShader::addInputVariable(const std::wstring& variableName, GlslVariable* variable)
{
	T_ASSERT(!m_inputVariables[variableName]);
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
	for (uint32_t i = m_variableScopes.back(); i < m_variables.size(); ++i)
	{
		const auto& v = m_variables[i];
		T_FATAL_ASSERT (v.outputPin != outputPin);
	}

	auto& v = m_variables.push_back();
	v.outputPin = outputPin;
	v.variable = new GlslVariable(outputPin->getNode(), variableName, type);
	return v.variable;
}

GlslVariable* GlslShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type)
{
	auto& v = m_outerVariables.push_back();
	v.outputPin = outputPin;
	v.variable = new GlslVariable(outputPin->getNode(), variableName, type);
	return v.variable;
}

//void GlslShader::associateVariable(const OutputPin* outputPin, GlslVariable* variable)
//{
//	m_variables.back().insert(std::make_pair(outputPin, variable));
//}

GlslVariable* GlslShader::getVariable(const OutputPin* outputPin)
{
	for (auto it = m_variables.rbegin(); it != m_variables.rend(); ++it)
	{
		if (it->outputPin == outputPin)
			return it->variable;
	}
	for (auto it = m_outerVariables.begin(); it != m_outerVariables.end(); ++it)
	{
		if (it->outputPin == outputPin)
			return it->variable;
	}
	return nullptr;
}

void GlslShader::pushScope()
{
	m_variableScopes.push_back((uint32_t)m_variables.size());
}

void GlslShader::popScope()
{
	m_variables.resize(m_variableScopes.back());
	m_variableScopes.pop_back();
}

//bool GlslShader::addUniform(const std::wstring& uniform)
//{
//	if (haveUniform(uniform))
//		return false;
//
//	m_uniforms.push_back(uniform);
//	return true;
//}
//
//const std::list< std::wstring >& GlslShader::getUniforms() const
//{
//	return m_uniforms;
//}
//
//bool GlslShader::haveUniform(const std::wstring& uniform) const
//{
//	return std::find(m_uniforms.begin(), m_uniforms.end(), uniform) != m_uniforms.end();
//}

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
	T_ASSERT(!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

const StringOutputStream& GlslShader::getOutputStream(BlockType blockType) const
{
	T_ASSERT(!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back());
}

std::wstring GlslShader::getGeneratedShader(const GlslLayout& layout) const
{
	StringOutputStream ss;

	ss << L"#version 450" << Endl;
	ss << L"#extension GL_ARB_separate_shader_objects : enable" << Endl;
	ss << L"#extension GL_ARB_shading_language_420pack : enable" << Endl;
	ss << Endl;

	if (m_shaderType == StFragment)
	{
		ss << L"layout (location = 0) out vec4 _gl_FragData_0;" << Endl;
		ss << L"layout (location = 1) out vec4 _gl_FragData_1;" << Endl;
		ss << L"layout (location = 2) out vec4 _gl_FragData_2;" << Endl;
		ss << L"layout (location = 3) out vec4 _gl_FragData_3;" << Endl;
		ss << Endl;
	}

	for (auto resource : layout.get())
	{
		if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
		{
			if (!uniformBuffer->get().empty())
			{
				ss << L"layout (std140, binding = " << uniformBuffer->getBinding() << L") uniform " << uniformBuffer->getName() << Endl;
				ss << L"{" << Endl;
				ss << IncreaseIndent;
				for (auto uniform : uniformBuffer->get())
					ss << glsl_type_name(uniform.type) << L" " << uniform.name << L";" << Endl;
				ss << DecreaseIndent;
				ss << L"};" << Endl;
				ss << Endl;
			}
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			ss << L"layout(set = 0, binding = " << texture->getBinding() << L") uniform texture2D " << texture->getName() << L";" << Endl;
			ss << Endl;
		}
		else if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			ss << L"layout(set = 0, binding = " << sampler->getBinding() << L") uniform sampler " << sampler->getName() << L";" << Endl;
			ss << Endl;
		}
	}

	//for (uint32_t frequency = UfOnce; frequency <= UfDraw; ++frequency)
	//{
	//	const auto& uniforms = cx.getScalarUniforms((UpdateFrequency)frequency);
	//	if (!uniforms.empty())
	//	{
	//		const wchar_t* c_uniformBufferNames[] = { L"UbOnce", L"UbFrame", L"UbDraw" };
	//		ss << L"layout (std140, binding = " << frequency << L") uniform " << c_uniformBufferNames[frequency] << Endl;
	//		ss << L"{" << Endl;
	//		ss << IncreaseIndent;
	//	
	//		for (auto uniform : uniforms.get())
	//			ss << uniform.type << L" " << uniform.name << L";" << Endl;

	//		ss << DecreaseIndent;
	//		ss << L"};" << Endl;
	//		ss << Endl;
	//	}
	//}

	//const auto& textureUniforms = cx.getTextureUniforms();
	//if (!textureUniforms.empty())
	//{
	//}

	//std::wstring texturesText = getOutputStream(BtTextures).str();
	//if (!texturesText.empty())
	//{
	//	ss << texturesText;
	//	ss << Endl;
	//}

	//std::wstring samplersText = getOutputStream(BtSamplers).str();
	//if (!samplersText.empty())
	//{
	//	ss << samplersText;
	//	ss << Endl;
	//}

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
