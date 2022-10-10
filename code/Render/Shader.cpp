#include "Render/Shader.h"
#include "Render/IProgram.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Shader", Shader, Object)

const Handle Shader::ms_default(L"Default");

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
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
	m_parameterBits.clear();
}

void Shader::getTechniques(SmallSet< handle_t >& outHandles) const
{
	for (const auto& technique : m_techniques)
		outHandles.insert(technique.first);
}

bool Shader::hasTechnique(handle_t handle) const
{
	return m_techniques.find(handle) != m_techniques.end();
}

void Shader::setCombination(handle_t handle, bool param, Permutation& inoutPermutation) const
{
	uint32_t bit = m_parameterBits[handle];
	if (param)
		inoutPermutation.combination |= bit;
	else
		inoutPermutation.combination &= ~bit;
}

Shader::Program Shader::getProgram(const Permutation& permutation) const
{
	const auto it = m_techniques.find(permutation.technique);
	if (it == m_techniques.end())
		return { nullptr, 0 };

	const auto& technique = it->second;
	for (const auto& combination : technique.combinations)
	{
		if ((permutation.combination & combination.mask) == combination.value)
			return { combination.program, combination.priority };
	}

	return { nullptr, 0 };
}

}
