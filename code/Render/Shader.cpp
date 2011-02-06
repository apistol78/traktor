#include "Render/Shader.h"
#include "Render/IProgram.h"
#include "Render/IRenderView.h"
#include "Render/Context/ProgramParameters.h"

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

	for (SmallMap< handle_t, Technique >::iterator i = m_techniques.begin(); i != m_techniques.end(); ++i)
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
	SmallMap< handle_t, Technique >::iterator i = m_techniques.find(handle);
	m_currentTechnique = (i != m_techniques.end()) ? &i->second : 0;
	updateCurrentProgram();
}

void Shader::setCombination(handle_t handle, bool param)
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
	if (m_currentProgram)
		m_currentProgram->setFloatParameter(handle, param);
}

void Shader::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	if (m_currentProgram)
		m_currentProgram->setFloatArrayParameter(handle, param, length);
}

void Shader::setVectorParameter(handle_t handle, const Vector4& param)
{
	if (m_currentProgram)
		m_currentProgram->setVectorParameter(handle, param);
}

void Shader::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	if (m_currentProgram)
		m_currentProgram->setVectorArrayParameter(handle, param, length);
}

void Shader::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	if (m_currentProgram)
		m_currentProgram->setMatrixParameter(handle, param);
}

void Shader::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	if (m_currentProgram)
		m_currentProgram->setMatrixArrayParameter(handle, param, length);
}

void Shader::setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& texture)
{
	SmallMap< handle_t, int32_t >::iterator i = m_textureMap.find(handle);
	if (i != m_textureMap.end())
		m_textureProxies[i->second] = texture;
	else
	{
		int32_t textureProxyId = int32_t(m_textureProxies.size());
		m_textureProxies.push_back(texture);
		m_textureMap.insert(handle, textureProxyId);
	}
}

void Shader::setStencilReference(uint32_t stencilReference)
{
	if (m_currentProgram)
		m_currentProgram->setStencilReference(stencilReference);
}

void Shader::draw(IRenderView* renderView, const Primitives& primitives)
{
	if (!m_currentProgram)
		return;

	for (SmallMap< handle_t, int32_t >::iterator i = m_textureMap.begin(); i != m_textureMap.end(); ++i)
	{
		resource::Proxy< ITexture >& textureProxy = m_textureProxies[i->second];
		if (textureProxy.validate())
			m_currentProgram->setTextureParameter(i->first, textureProxy);
		else
			m_currentProgram->setTextureParameter(i->first, 0);
	}

	renderView->setProgram(m_currentProgram);
	renderView->draw(primitives);
}

IProgram* Shader::getCurrentProgram() const
{
	return m_currentProgram;
}

void Shader::setProgramParameters(ProgramParameters* programParameters)
{
	for (SmallMap< handle_t, int32_t >::iterator i = m_textureMap.begin(); i != m_textureMap.end(); ++i)
	{
		resource::Proxy< ITexture >& textureProxy = m_textureProxies[i->second];
		if (textureProxy.validate())
			programParameters->setTextureParameter(i->first, textureProxy);
		else
			programParameters->setTextureParameter(i->first, 0);
	}
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
			m_currentProgram = i->program;
			break;
		}
	}
}

	}
}
