#include "Render/Types.h"
#include "Render/Vulkan/CubeTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureVk", CubeTextureVk, ICubeTexture)

CubeTextureVk::CubeTextureVk()
{
}

CubeTextureVk::~CubeTextureVk()
{
	destroy();
}

bool CubeTextureVk::create(const CubeTextureCreateDesc& desc)
{
	return true;
}

void CubeTextureVk::destroy()
{
}

ITexture* CubeTextureVk::resolve()
{
	return this;
}

int32_t CubeTextureVk::getMips() const
{
	return 0;
}

int32_t CubeTextureVk::getSide() const
{
	return 0;
}

bool CubeTextureVk::lock(int32_t side, int32_t level, Lock& lock)
{
	return true;
}

void CubeTextureVk::unlock(int32_t side, int32_t level)
{
}

	}
}
