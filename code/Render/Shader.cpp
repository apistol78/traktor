#include "Render/Shader.h"
#include "Render/Program.h"
#include "Render/RenderView.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Shader", Shader, Object)

Shader::Shader()
:	m_parameterValue(0)
,	m_currentTechnique(0)
,	m_currentProgram(0)
{
}

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
	m_currentTechnique = 0;
	m_currentProgram = 0;

	for (std::map< handle_t, Technique >::iterator i = m_techniques.begin(); i != m_techniques.end(); ++i)
	{
		for (std::vector< Combination >::iterator j = i->second.combinations.begin(); j != i->second.combinations.end(); ++j)
		{
			if (j->program)
			{
				j->program->destroy();
				j->program = 0;
			}
		}
		i->second.combinations.resize(0);
	}
	m_techniques.clear();
}

bool Shader::hasTechnique(handle_t handle) const
{
	return m_techniques.find(handle) != m_techniques.end();
}

void Shader::setTechnique(handle_t handle)
{
	std::map< handle_t, Technique >::iterator i = m_techniques.find(handle);
	m_currentTechnique = (i != m_techniques.end()) ? &i->second : 0;
	updateCurrentProgram();
}

void Shader::setBooleanParameter(handle_t handle, bool param)
{
	uint32_t bit = m_parameterBits[handle];
	if (param)
		m_parameterValue |= bit;
	else
		m_parameterValue &= ~bit;
	updateCurrentProgram();
}

void Shader::setFloatParameter(handle_t handle, float param)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setFloatParameter(handle, param);
}

void Shader::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setFloatArrayParameter(handle, param, length);
}

void Shader::setVectorParameter(handle_t handle, const Vector4& param)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setVectorParameter(handle, param);
}

void Shader::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setVectorArrayParameter(handle, param, length);
}

void Shader::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setMatrixParameter(handle, param);
}

void Shader::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setMatrixArrayParameter(handle, param, length);
}

void Shader::setSamplerTexture(handle_t handle, Texture* texture)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setSamplerTexture(handle, texture);
}

void Shader::setStencilReference(uint32_t stencilReference)
{
	if (!m_currentTechnique)
		return;

	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
		i->program->setStencilReference(stencilReference);
}

void Shader::draw(RenderView* renderView, const Primitives& primitives)
{
	if (!m_currentProgram)
		return;

	renderView->setProgram(m_currentProgram);
	renderView->draw(primitives);
}

bool Shader::isOpaque() const
{
	if (!m_currentProgram)
		return true;

	return m_currentProgram->isOpaque();
}

void Shader::updateCurrentProgram()
{
	m_currentProgram = 0;

	if (!m_currentTechnique)
		return;

	uint32_t value = m_parameterValue & m_currentTechnique->parameterMask;
	for (std::vector< Combination >::iterator i = m_currentTechnique->combinations.begin(); i != m_currentTechnique->combinations.end(); ++i)
	{
		if (i->parameterValue == value)
		{
			m_currentProgram  = i->program;
			break;
		}
	}
}

	}
}
