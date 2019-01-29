#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslShader.h"

namespace traktor
{
	namespace render
	{

GlslShader::GlslShader(ShaderType shaderType)
:	m_shaderType(shaderType)
,	m_nextTemporaryVariable(0)
{
	pushScope();
	pushOutputStream(BtUniform, new StringOutputStream());
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
	popOutputStream(BtUniform);
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
	if (!m_variables.empty())
		m_variables.pop_back();
}

void GlslShader::addUniform(const std::wstring& uniform)
{
	m_uniforms.insert(uniform);
}

const std::set< std::wstring >& GlslShader::getUniforms() const
{
	return m_uniforms;
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
	if (!m_outputStreams[int(blockType)].empty())
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

std::wstring GlslShader::getGeneratedShader(const PropertyGroup* settings, const std::wstring& name, const GlslRequirements& requirements) const
{
	StringOutputStream ss;

	if (getOutputStream(BtBody).empty())
		return L"";

	if (m_shaderType == StVertex || m_shaderType == StFragment)
	{
		ss << L"#version 150" << Endl;
		ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
		if (!name.empty())
			ss << L"// " << name << Endl;
		ss << Endl;

		ss << L"precision highp float;" << Endl;
		ss << Endl;

		ss << L"uniform vec2 _gl_targetSize;" << Endl;
		ss << Endl;
	}
	else if (m_shaderType == StCompute)
	{
		ss << L"#version 430" << Endl;
		ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
		if (!name.empty())
			ss << L"// " << name << Endl;
		ss << Endl;

		ss << L"layout(local_size_x = 1, local_size_y = 1) in;" << Endl;
		ss << Endl;
	}

	if (m_shaderType == StVertex)
	{
		// Add post-scale transform function.
		ss << L"vec4 PV(in vec4 cp0)" << Endl;
		ss << L"{" << Endl;
		ss << L"\treturn vec4(cp0.x, -cp0.y, cp0.z, cp0.w);" << Endl;
		ss << L"}" << Endl;
		ss << Endl;

		// Add bilinear texture fetch.
		if (requirements.vertexBilinearSampler)
		{
			ss << L"vec4 texture2DBilinear(sampler2D sampler, vec2 uv)" << Endl;
			ss << L"{" << Endl;
			ss << L"\tvec2 textureSize = vec2(textureSize(sampler, 0));" << Endl;
			ss << L"\tvec2 texelSize = 1.0 / textureSize;" << Endl;
			ss << L"\tvec4 tl = texture(sampler, uv);" << Endl;
			ss << L"\tvec4 tr = texture(sampler, uv + vec2(texelSize.x, 0.0));" << Endl;
			ss << L"\tvec4 bl = texture(sampler, uv + vec2(0.0, texelSize.y));" << Endl;
			ss << L"\tvec4 br = texture(sampler, uv + texelSize);" << Endl;
			ss << L"\tvec2 f = fract(uv * textureSize);" << Endl;
			ss << L"\tvec4 a = mix(tl, tr, f.x);" << Endl;
			ss << L"\tvec4 b = mix(bl, br, f.x);" << Endl;
			ss << L"\treturn mix(a, b, f.y);" << Endl;
			ss << L"}" << Endl;
			ss << Endl;
		}
	}
	else if (m_shaderType == StFragment)
	{
		// Add fragment outputs.
		ss << L"out vec4 _gl_FragData_0;" << Endl;
		ss << L"out vec4 _gl_FragData_1;" << Endl;
		ss << L"out vec4 _gl_FragData_2;" << Endl;
		ss << L"out vec4 _gl_FragData_3;" << Endl;
	}

	ss << getOutputStream(BtUniform).str();
	ss << Endl;
	ss << getOutputStream(BtInput).str();
	ss << Endl;
	ss << getOutputStream(BtOutput).str();
	ss << Endl;
	ss << getOutputStream(BtScript).str();
	ss << Endl;
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
