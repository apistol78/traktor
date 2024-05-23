/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Glsl/GlslImage.h"
#include "Render/Editor/Glsl/GlslLayout.h"
#include "Render/Editor/Glsl/GlslSampler.h"
#include "Render/Editor/Glsl/GlslShader.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Editor/Glsl/GlslTexture.h"
#include "Render/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor::render
{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_temporaryVariableAlloc(0, 65535)
{
	pushScope();
	pushOutputStream(BtInput, T_FILE_LINE_W);
	pushOutputStream(BtOutput, T_FILE_LINE_W);
	pushOutputStream(BtBody, T_FILE_LINE_W);
}

GlslShader::~GlslShader()
{
	popOutputStream(BtBody);
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
	const int32_t index = (int32_t)m_temporaryVariableAlloc.alloc();
	const std::wstring name = str(L"v%d", index);

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

void GlslShader::addModule(const Guid& moduleId)
{
	m_moduleIds.insert(moduleId);
}

std::wstring GlslShader::getGeneratedShader(
	const PropertyGroup* settings,
	const GlslLayout& layout,
	const GlslRequirements& requirements,
	const std::function< std::wstring(const Guid&) >& resolveModule
) const
{
	StringOutputStream ss;

	ss << L"#version 460" << Endl;
	ss << L"#extension GL_ARB_separate_shader_objects : enable" << Endl;
	ss << L"#extension GL_ARB_shading_language_420pack : enable" << Endl;
	ss << L"#extension GL_EXT_samplerless_texture_functions : enable" << Endl;
	ss << L"#extension GL_EXT_nonuniform_qualifier : enable" << Endl;

	const bool supportControlFlowAttributes = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.ControlFlowAttributes", true) : true);
	if (supportControlFlowAttributes)
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

	PrecisionHint precisionHint = PrecisionHint::Undefined;
	const bool ignorePrecisionHint = (settings != nullptr ? settings->getProperty< bool >(L"Glsl.Vulkan.IgnorePrecisionHint", false) : false);
	if (!ignorePrecisionHint)
	{
		if (m_shaderType == StVertex)
			precisionHint = requirements.vertexPrecisionHint;
		else if (m_shaderType == StFragment)
			precisionHint = requirements.fragmentPrecisionHint;
	}
	
	switch (precisionHint)
	{
	case PrecisionHint::Low:
		ss << L"precision lowp float;" << Endl;
		ss << Endl;
		break;

	case PrecisionHint::Medium:
		ss << L"precision mediump float;" << Endl;
		ss << Endl;
		break;

	case PrecisionHint::High:
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

	uint8_t stageMask = 0;
	if (m_shaderType == StVertex)
		stageMask = GlslResource::BsVertex;
	else if (m_shaderType == StFragment)
		stageMask = GlslResource::BsFragment;
	else if (m_shaderType == StCompute)
		stageMask = GlslResource::BsCompute;

	if (requirements.useTargetSize)
	{
		ss << L"// Push constants." << Endl;
		ss << L"layout( push_constant ) uniform constants" << Endl;
		ss << L"{" << Endl;
		ss << IncreaseIndent;
		ss << L"vec4 _vk_targetSize;" << Endl;
		ss << DecreaseIndent;
		ss << L"};" << Endl;
		ss << Endl;
	}

	if (layout.count< GlslUniformBuffer >(stageMask) > 0)
	{
		ss << L"// Uniform buffers." << Endl;
		for (auto resource : layout.getByStage(stageMask))
		{
			if (resource->getBinding() < 0)
				continue;

			if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
			{
				if (!uniformBuffer->get().empty())
				{
					ss << L"layout (std140, binding = " << uniformBuffer->getBinding() << L", set = " << (int32_t)uniformBuffer->getSet() << L") uniform " << uniformBuffer->getName() << Endl;
					ss << L"{" << Endl;
					ss << IncreaseIndent;
					for (auto uniform : uniformBuffer->get())
					{
						// Force high precision on uniform blocks since they share signature.
						if (uniform.type >= GlslType::Float && uniform.type <= GlslType::Float4x4)
							ss << L"highp ";
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
		}
	}

	if (layout.count< GlslTexture >(stageMask) > 0)
	{
		ss << L"// Textures" << Endl;
		for (auto resource : layout.getByStage(stageMask))
		{
			if (resource->getBinding() < 0)
				continue;

			if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
			{
				switch (texture->getUniformType())
				{
				case GlslType::Texture2D:
					ss << L"layout (binding = " << texture->getBinding() << L", set = " << (int32_t)texture->getSet() << L") uniform texture2D " << texture->getName() << (texture->isIndexed() ? L"[];" : L";") << Endl;
					break;

				case GlslType::Texture3D:
					ss << L"layout (binding = " << texture->getBinding() << L", set = " << (int32_t)texture->getSet() << L") uniform texture3D " << texture->getName() << (texture->isIndexed() ? L"[];" : L";") << Endl;
					break;

				case GlslType::TextureCube:
					ss << L"layout (binding = " << texture->getBinding() << L", set = " << (int32_t)texture->getSet() << L") uniform textureCube " << texture->getName() << (texture->isIndexed() ? L"[];" : L";") << Endl;
					break;

				default:
					break;
				}
			}
		}
		ss << Endl;
	}

	if (layout.count< GlslSampler >(stageMask) > 0)
	{
		ss << L"// Samplers" << Endl;
		for (auto resource : layout.getByStage(stageMask))
		{
			if (resource->getBinding() < 0)
				continue;

			if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
			{
				if (sampler->getState().compare == CompareFunction::None)
					ss << L"layout (binding = " << sampler->getBinding() << L", set = " << (int32_t)sampler->getSet() << L") uniform sampler " << sampler->getName() << L"; " << Endl;
				else
					ss << L"layout (binding = " << sampler->getBinding() << L", set = " << (int32_t)sampler->getSet() << L") uniform samplerShadow " << sampler->getName() << L";" << Endl;
			}
		}
		ss << Endl;
	}

	 if (layout.count< GlslImage >(stageMask) > 0)
	{
		ss << L"// Images" << Endl;
		 for (auto resource : layout.getByStage(stageMask))
		 {
			 if (resource->getBinding() < 0)
				 continue;

		 	if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		 	{
		 		switch (image->getUniformType())
		 		{
		 		case GlslType::Image2D:
		 			ss << L"layout (binding = " << image->getBinding() << L", set = " << (int32_t)image->getSet() << L", rgba32f) uniform image2D " << image->getName() << (image->isIndexed() ? L"[];" : L";") << Endl;
		 			break;

		 		case GlslType::Image3D:
		 			ss << L"layout (binding = " << image->getBinding() << L", set = " << (int32_t)image->getSet() << L", rgba32f) uniform image3D " << image->getName() << (image->isIndexed() ? L"[];" : L";") << Endl;
		 			break;

		 		case GlslType::ImageCube:
		 			ss << L"layout (binding = " << image->getBinding() << L", set = " << (int32_t)image->getSet() << L", rgba32f) uniform imageCube " << image->getName() << (image->isIndexed() ? L"[];" : L";") << Endl;
		 			break;

		 		default:
		 			break;
		 		}
		 	}
		 }
		 ss << Endl;
	}

	if (layout.count< GlslStorageBuffer >(stageMask) > 0)
	{
		ss << L"// Storage buffers." << Endl;
		for (auto resource : layout.getByStage(stageMask))
		{
			if (resource->getBinding() < 0)
				continue;

			if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
			{
				ss << L"struct " << storageBuffer->getName() << L"_Type" << Endl;
				ss << L"{" << Endl;
				ss << IncreaseIndent;
				for (auto element : storageBuffer->get())
				{
					// Force high precision on SSBO since they share signature.
					if (element.type >= DtFloat1 && element.type <= DtFloat4)
						ss << L"highp ";
					ss << glsl_storage_type(element.type) << L" " << element.name << L";" << Endl;
				}
				ss << DecreaseIndent;
				ss << L"};" << Endl;
				ss << Endl;
				if (m_shaderType != StCompute)
					ss << L"layout (std430, binding = " << storageBuffer->getBinding() << L", set = " << (int32_t)storageBuffer->getSet() << L") readonly buffer " << storageBuffer->getName() << Endl;
				else
					ss << L"layout (std430, binding = " << storageBuffer->getBinding() << L", set = " << (int32_t)storageBuffer->getSet() << L") buffer " << storageBuffer->getName() << Endl;
				ss << L"{" << Endl;
				ss << IncreaseIndent;
				ss << storageBuffer->getName() << L"_Type " << storageBuffer->getName() << L"_Data[];" << Endl;
				ss << DecreaseIndent;
				ss << L"};" << Endl;
				ss << Endl;
			}
		}
	}

	const std::wstring inputText = getOutputStream(BtInput).str();
	if (!inputText.empty())
	{
		ss << L"// Inputs" << Endl;
		ss << inputText;
		ss << Endl;
	}

	const std::wstring outputText = getOutputStream(BtOutput).str();
	if (!outputText.empty())
	{
		ss << L"// Outputs" << Endl;
		ss << outputText;
		ss << Endl;
	}

	if (!m_moduleIds.empty())
	{
		ss << L"// Modules" << Endl;
		for (const auto& moduleId : m_moduleIds)
		{
			const std::wstring moduleText = resolveModule(moduleId);
			if (!moduleText.empty())
			{
				ss << moduleText << Endl;
				ss << Endl;
			}
		}
	}

	if (m_shaderType == StCompute)
	{
		const int32_t* ls = requirements.localSize;
		ss << L"layout(local_size_x = " << ls[0] << L", local_size_y = " << ls[1] << L", local_size_z = " << ls[2] << L") in;" << Endl;
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
