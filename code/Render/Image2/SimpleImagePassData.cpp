#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Image2/SimpleImagePass.h"
#include "Render/Image2/SimpleImagePassData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SimpleImagePassData", 0, SimpleImagePassData, IImagePassData)

Ref< IImagePass > SimpleImagePassData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< SimpleImagePass > instance = new SimpleImagePass();

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	for (const auto& source : m_sources)
	{
		instance->m_sources.push_back({
			getParameterHandle(source.input),
			getParameterHandle(source.parameter)
		});
	}

	// Get output handle.
	instance->m_output = getParameterHandle(m_output);
	return instance; 
}

void SimpleImagePassData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< std::wstring >(L"output", m_output);
}

void SimpleImagePassData::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"input", input);
	s >> Member< std::wstring >(L"parameter", parameter);
}

    }
}