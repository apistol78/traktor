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

int CubeTextureVk::getWidth() const
{
	return 0;
}

int CubeTextureVk::getHeight() const
{
	return 0;
}

int CubeTextureVk::getDepth() const
{
	return 0;
}

bool CubeTextureVk::lock(int side, int level, Lock& lock)
{
	return true;
}

void CubeTextureVk::unlock(int side, int level)
{
}

	}
}
