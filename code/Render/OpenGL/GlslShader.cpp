#include "Render/OpenGL/GlslShader.h"

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
	pushOutputStream(BtBody, new StringOutputStream());
}

GlslShader::~GlslShader()
{
	popOutputStream(BtBody);
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
	T_ASSERT (!m_variables.empty());
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

std::wstring GlslShader::getGeneratedShader()
{
	StringOutputStream ss;

	ss << L"// THIS SHADER IS AUTOMATICALLY GENERATED! DO NOT EDIT!" << Endl;
	ss << Endl;

#if defined(T_OPENGL_ES2)
	ss << L"precision mediump float;" << Endl;
	ss << Endl;

	if (m_shaderType == StVertex)
	{
		// Add post-orientation transform function.
		ss << L"uniform vec4 t_internal_postOrientationCoeffs;" << Endl;
		ss << Endl;
		ss << L"vec4 PV(in vec4 cp0)" << Endl;
		ss << L"{" << Endl;
		ss << L"\treturn vec4(" << Endl;
		ss << L"\t\tcp0.x * t_internal_postOrientationCoeffs.x + cp0.y * t_internal_postOrientationCoeffs.y," << Endl;
		ss << L"\t\tcp0.x * t_internal_postOrientationCoeffs.z + cp0.y * t_internal_postOrientationCoeffs.w," << Endl;
		ss << L"\t\tcp0.z," << Endl;
		ss << L"\t\tcp0.w" << Endl;
		ss << L"\t);" << Endl;
		ss << L"}" << Endl;
		ss << Endl;
	}
#else
	ss << L"#version 120" << Endl;
	ss << Endl;
#endif

#if defined(T_OPENGL_ES2)
	ss << L"uniform vec4 _gl_targetSize;" << Endl;
#else
	ss << L"uniform vec2 _gl_targetSize;" << Endl;
#endif
	ss << Endl;

	ss << getOutputStream(BtUniform).str();
	ss << Endl;
	ss <<getOutputStream(BtInput).str();
	ss << Endl;
	ss << getOutputStream(BtOutput).str();
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
