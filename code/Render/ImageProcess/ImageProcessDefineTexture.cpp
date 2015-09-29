#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessDefineTexture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessDefineTexture", 0, ImageProcessDefineTexture, ImageProcessDefine)

bool ImageProcessDefineTexture::define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	resource::Proxy< ITexture > texture;

	if (!resourceManager->bind(m_texture, texture))
		return false;

	imageProcess->setTextureParameter(getParameterHandle(m_parameter), texture);
	return true;
}

void ImageProcessDefineTexture::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"parameter", m_parameter);
	s >> resource::Member< ITexture >(L"texture", m_texture);
}

	}
}
