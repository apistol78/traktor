#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Vulkan/Editor/Glsl/GlslImage.h"
#include "Render/Vulkan/Editor/Glsl/GlslLayout.h"
#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"
#include "Render/Vulkan/Editor/Glsl/GlslShader.h"
#include "Render/Vulkan/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"
#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_temporaryVariableAlloc(0, 65535)
{
	pushScope();
	pushOutputStream(BtInput, T_FILE_LINE_W);
	pushOutputStream(BtOutput, T_FILE_LINE_W);
	pushOutputStream(BtScript, T_FILE_LINE_W);
	pushOutputStream(BtBody, T_FILE_LINE_W);
}

GlslShader::~GlslShader()
{
	popOutputStream(BtBody);
	popOutputStream(BtScript);
	popOutputStream(BtOutput);
	popOutputStream(BtInput);
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
	int32_t index = (int32_t)m_temporaryVariableAlloc.alloc();
	std::wstring name = str(L"v%d", index);

	auto& v = m_variables.push_back();
	v.outputPin = outputPin;
	v.variable = new GlslVariable(outputPin->getNode(), name, type);
	v.index = index;
	return v.variable;
}

GlslVariable* GlslShader::createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type)
{
#if defined(_DEBUG)
	for (uint32_t i = m_variableScopes.back(); i < m_variables.size(); ++i)
	{
		const auto& v = m_variables[i];
		T_FATAL_ASSERT (v.outputPin != outputPin);
	}
#endif

	auto& v = m_variables.push_back();
	v.outputPin = outputPin;
	v.variable = new GlslVariable(outputPin->getNode(), variableName, type);
	v.index = -1;
	return v.variable;
}

GlslVariable* GlslShader::createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type)
{
	auto& v = m_outerVariables.push_back();
	v.outputPin = outputPin;
	v.variable = new GlslVariable(outputPin->getNode(), variableName, type);
	v.index = -1;
	return v.variable;
}

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
	// Free all indices used for temporary variables within scope to be popped.
	for (size_t i = m_variableScopes.back(); i < m_variables.size(); ++i)
	{
		int32_t index = m_variables[i].index;
		if (index >= 0)
			m_temporaryVariableAlloc.free(index);
	}

	m_variables.resize(m_variableScopes.back());
	m_variableScopes.pop_back();
}

bool GlslShader::defineScript(const std::wstring& signature)
{
	std::set< std::wstring >::iterator i = m_scriptSignatures.find(signature);
	if (i != m_scriptSignatures.end())
		return false;

	m_scriptSignatures.insert(signature);
	return true;
}

StringOutputStream& GlslShader::pushOutputStream(BlockType blockType, const wchar_t* const tag)
{
	Ref< StringOutputStream > os = new StringOutputStream();
	m_outputStreams[int(blockType)].push_back({ os, tag });
	return *os;
}

void GlslShader::popOutputStream(BlockType blockType)
{
	if (!m_outputStreams[int(blockType)].empty())
		m_outputStreams[int(blockType)].pop_back();
}

StringOutputStream& GlslShader::getOutputStream(BlockType blockType)
{
	T_ASSERT(!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back().outputStream);
}

const StringOutputStream& GlslShader::getOutputStream(BlockType blockType) const
{
	T_ASSERT(!m_outputStreams[int(blockType)].empty());
	return *(m_outputStreams[int(blockType)].back().outputStream);
}

std::wstring GlslShader::getGeneratedShader(const PropertyGroup* settings, const GlslLayout& layout, const GlslRequirements& requirements) const
{
	StringOutputStream ss;

	if (getOutputStream(BtBody).empty())
		return L"";

	ss << L"#version 450" << Endl;
	ss << L"#extension GL_ARB_separate_shader_objects : enable" << Endl;
	ss << L"#extension GL_ARB_shading_language_420pack : enable" << Endl;
	ss << L"#extension GL_EXT_samplerless_texture_functions : enable" << Endl;
	ss << L"#extension GL_EXT_control_flow_attributes : enable" << Endl;

	const bool supportBallot = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.Ballot", true) : true);
	if (supportBallot)
		ss << L"#extension GL_ARB_shader_ballot : enable" << Endl;

	const bool supportStorageTypes = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.StorageTypes", true) : true);
	if (supportStorageTypes)
	{
		ss << L"#extension GL_EXT_shader_8bit_storage : enable" << Endl;
		ss << L"#extension GL_EXT_shader_16bit_storage : enable" << Endl;
	}

	const bool supportExplicitArithmeticTypes = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.ExplicitArithmeticTypes", true) : true);
	if (supportExplicitArithmeticTypes)
	{
		ss << L"#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable" << Endl;
		ss << L"#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable" << Endl;
		ss << L"#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable" << Endl;
	}

	ss << Endl;

	PrecisionHint precisionHint = PhUndefined;
	if (m_shaderType == StVertex)
		precisionHint = requirements.vertexPrecisionHint;
	else if (m_shaderType == StFragment)
		precisionHint = requirements.fragmentPrecisionHint;

	switch (precisionHint)
	{
	case PhLow:
		ss << L"precision lowp float;" << Endl;
		ss << Endl;
		break;

	case PhMedium:
		ss << L"precision mediump float;" << Endl;
		ss << Endl;
		break;

	case PhHigh:
		ss << L"precision highp float;" << Endl;
		ss << Endl;
		break;

	default:
		break;
	}

	// Ensure output position is invariant since we're using multiple passes which
	// expect positions to match up.
	if (m_shaderType == StVertex)
	{
		ss << L"invariant gl_Position;" << Endl;
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
				{
					if (uniform.length <= 1)
						ss << glsl_type_name(uniform.type) << L" " << uniform.name << L";" << Endl;
					else
						ss << glsl_type_name(uniform.type) << L" " << uniform.name << L"[" << uniform.length << L"];" << Endl;
				}
				ss << DecreaseIndent;
				ss << L"};" << Endl;
				ss << Endl;
			}
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			switch (texture->getUniformType())
			{
			case GtTexture2D:
				ss << L"layout(set = 0, binding = " << texture->getBinding() << L") uniform texture2D " << texture->getName() << L";" << Endl;
				break;

			case GtTexture3D:
				ss << L"layout(set = 0, binding = " << texture->getBinding() << L") uniform texture3D " << texture->getName() << L";" << Endl;
				break;

			case GtTextureCube:
				ss << L"layout(set = 0, binding = " << texture->getBinding() << L") uniform textureCube " << texture->getName() << L";" << Endl;
				break;

			default:
				break;
			}
			ss << Endl;
		}
		else if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			if (sampler->getState().compare == CfNone)
				ss << L"layout(set = 0, binding = " << sampler->getBinding() << L") uniform sampler " << sampler->getName() << L";" << Endl;
			else
				ss << L"layout(set = 0, binding = " << sampler->getBinding() << L") uniform samplerShadow " << sampler->getName() << L";" << Endl;
			ss << Endl;
		}
		else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		{
			ss << L"layout(set = 0, binding = " << image->getBinding() << L", rgba32f) uniform image2D " << image->getName() << L";" << Endl;
			ss << Endl;
		}
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			ss << L"struct " << storageBuffer->getName() << L"_Type" << Endl;
			ss << L"{" << Endl;
			ss << IncreaseIndent;
			for (auto element : storageBuffer->get())
				ss << glslStorageType(element.type) << L" " << element.name << L";" << Endl;
			ss << DecreaseIndent;
			ss << L"};" << Endl;
			ss << Endl;
			if (m_shaderType != StCompute)
				ss << L"layout (std140, binding = " << storageBuffer->getBinding() << L") readonly buffer " << storageBuffer->getName() << Endl;
			else
				ss << L"layout (std140, binding = " << storageBuffer->getBinding() << L") buffer " << storageBuffer->getName() << Endl;
			ss << L"{" << Endl;
			ss << IncreaseIndent;
			ss << storageBuffer->getName() << L"_Type " << storageBuffer->getName() << L"_Data[];" << Endl;
			ss << DecreaseIndent;
			ss << L"};" << Endl;
			ss << Endl;
		}
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
