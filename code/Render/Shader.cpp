#include "Render/Shader.h"
#include "Render/IProgram.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Shader", Shader, Object)

Shader::Shader()
:	m_parameterValue(0)
,	m_currentTechnique(nullptr)
,	m_currentProgram(nullptr)
,	m_currentPriority(0)
{
}

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
	m_currentTechnique = nullptr;
	m_currentProgram = nullptr;
	m_currentPriority = 0;

	for (auto& technique : m_techniques)
	{
		for (auto& combination : technique.second.combinations)
		{
			if (combination.program)
				combination.program->destroy();
		}
		technique.second.combinations.resize(0);
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
	m_currentTechnique = (i != m_techniques.end()) ? &i->second : nullptr;
	updateCurrentProgram();
}

void Shader::getTechniques(SmallSet< handle_t >& outHandles) const
{
	for (const auto& technique : m_techniques)
		outHandles.insert(technique.first);
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

void Shader::setTextureParameter(handle_t handle, ITexture* texture)
{
	if (m_currentProgram)
		m_currentProgram->setTextureParameter(handle, texture);
}

void Shader::setStencilReference(uint32_t stencilReference)
{
	if (m_currentProgram)
		m_currentProgram->setStencilReference(stencilReference);
}

void Shader::draw(IRenderView* renderView, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const Primitives& primitives)
{
	if (!m_currentProgram)
		return;

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		m_currentProgram,
		primitives
	);
}

void Shader::draw(IRenderView* renderView, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const Primitives& primitives, uint32_t instanceCount)
{
	if (!m_currentProgram)
		return;

	renderView->draw(
		vertexBuffer,
		indexBuffer,
		m_currentProgram,
		primitives,
		instanceCount
	);
}

void Shader::updateCurrentProgram()
{
	m_currentProgram = nullptr;
	m_currentPriority = 0;

	if (!m_currentTechnique)
		return;

	for (const auto& combination : m_currentTechnique->combinations)
	{
		if ((m_parameterValue & combination.mask) == combination.value)
		{
			m_currentProgram = combination.program;
			m_currentPriority = combination.priority;
			break;
		}
	}
}

	}
}
