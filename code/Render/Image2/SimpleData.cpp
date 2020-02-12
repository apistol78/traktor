#include "Render/Shader.h"
#include "Render/Image2/Simple.h"
#include "Render/Image2/SimpleData.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SimpleData", 0, SimpleData, ImageStepData)

Ref< const ImageStep > SimpleData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* /*renderSystem*/) const
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

    }
}