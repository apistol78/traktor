#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Image2/Simple.h"
#include "Render/Image2/SimpleData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SimpleData", 0, SimpleData, IImageStepData)

Ref< const IImageStep > SimpleData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* /*renderSystem*/) const
{
	Ref< Simple > instance = new Simple();

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	for (const auto& source : m_sources)
	{
		instance->m_sources.push_back({
			getParameterHandle(source.textureId),
			getParameterHandle(source.parameter)
		});
	}

	return instance; 
}

void SimpleData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

void SimpleData::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"textureId", textureId);
	s >> Member< std::wstring >(L"parameter", parameter);
}

    }
}