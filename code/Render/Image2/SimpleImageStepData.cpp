#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Image2/SimpleImageStep.h"
#include "Render/Image2/SimpleImageStepData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SimpleImageStepData", 0, SimpleImageStepData, IImageStepData)

Ref< const IImageStep > SimpleImageStepData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< SimpleImageStep > instance = new SimpleImageStep();

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

	return instance; 
}

void SimpleImageStepData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

void SimpleImageStepData::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"input", input);
	s >> Member< std::wstring >(L"parameter", parameter);
}

    }
}