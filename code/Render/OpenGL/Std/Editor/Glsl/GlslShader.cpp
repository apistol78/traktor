#include "Render/Editor/OutputPin.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslImage.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslLayout.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslSampler.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslShader.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslTexture.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_nextTemporaryVariable(0)
{
	pushScope();
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

	if (getOutputStream(BtBody).empty())
		return L"";

	ss << L"#version 430" << Endl;
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
		else if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			const auto texture = layout.get< GlslTexture >([&](const GlslTexture* resource) {
				return resource->getName() == sampler->getTextureName();
			});
			if (!texture)
				return L"";

			if (sampler->getState().compare == CfNone)
			{
				switch (texture->getUniformType())
				{
				case GtTexture2D:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform sampler2D " << sampler->getName() << L";" << Endl;
					break;

				case GtTexture3D:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform sampler3D " << sampler->getName() << L";" << Endl;
					break;

				case GtTextureCube:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform samplerCube " << sampler->getName() << L";" << Endl;
					break;
				}
			}
			else
			{
				switch (texture->getUniformType())
				{
				case GtTexture2D:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform sampler2DShadow " << sampler->getName() << L";" << Endl;
					break;

				case GtTexture3D:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform sampler3DShadow " << sampler->getName() << L";" << Endl;
					break;

				case GtTextureCube:
					ss << L"layout (binding = " << sampler->getUnit() << L") uniform samplerCubeShadow " << sampler->getName() << L";" << Endl;
					break;
				}				
			}

			ss << Endl;
		}
		// else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		// {
		// 	ss << L"layout(binding = " << image->getBinding() << L", rgba32f) uniform image2D " << image->getName() << L";" << Endl;
		// 	ss << Endl;
		// }
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			uint32_t binding = layout.typedIndexOf< GlslStorageBuffer >(storageBuffer);

			ss << L"struct " << storageBuffer->getName() << L"_Type" << Endl;
			ss << L"{" << Endl;
			ss << IncreaseIndent;
			for (auto element : storageBuffer->get())
				ss << glsl_type_name(element.type) << L" " << element.name << L";" << Endl;
			ss << DecreaseIndent;
			ss << L"};" << Endl;
			ss << Endl;
// #if defined(T_ENABLE_OPENGL_STORAGE_BUFFERS)
			ss << L"layout (std140, binding = " << (binding + 3) << L") buffer " << storageBuffer->getName() << Endl;
// #else
// 			ss << L"layout (std140, binding = " << (binding + 3) << L") uniform " << storageBuffer->getName() << Endl;
// #endif
			ss << L"{" << Endl;
			ss << IncreaseIndent;
// #if defined(T_ENABLE_OPENGL_STORAGE_BUFFERS)
			ss << storageBuffer->getName() << L"_Type " << storageBuffer->getName() << L"_Data[];" << Endl;
// #else
// 			ss << storageBuffer->getName() << L"_Type " << storageBuffer->getName() << L"_Data[1024];" << Endl;
// #endif
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
