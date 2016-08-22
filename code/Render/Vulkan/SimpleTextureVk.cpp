#include "Render/Vulkan/SimpleTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVk", SimpleTextureVk, ISimpleTexture)

SimpleTextureVk::SimpleTextureVk()
{
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
}

bool SimpleTextureVk::create(const SimpleTextureCreateDesc& desc)
{
	return true;
}

void SimpleTextureVk::destroy()
{
}

ITexture* SimpleTextureVk::resolve()
{
	return this;
}

int SimpleTextureVk::getWidth() const
{
	return 0;
}

int SimpleTextureVk::getHeight() const
{
	return 0;
}

bool SimpleTextureVk::lock(int level, Lock& lock)
{
	return true;
}

void SimpleTextureVk::unlock(int level)
{
}

void* SimpleTextureVk::getInternalHandle()
{
	return 0;
}

	}
}
