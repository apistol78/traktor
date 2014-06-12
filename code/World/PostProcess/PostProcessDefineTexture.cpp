#pragma optimize( "", off )

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessDefineTexture.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessDefineTexture", 0, PostProcessDefineTexture, PostProcessDefine)

bool PostProcessDefineTexture::define(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	resource::Proxy< render::ITexture > texture;

	if (!resourceManager->bind(m_texture, texture))
		return false;

	postProcess->setTextureParameter(render::getParameterHandle(m_parameter), texture);
	return true;
}

void PostProcessDefineTexture::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"parameter", m_parameter);
	s >> resource::Member< render::ITexture >(L"texture", m_texture);
}

	}
}
