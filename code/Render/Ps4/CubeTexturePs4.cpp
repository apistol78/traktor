#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/CubeTexturePs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTexturePs4", CubeTexturePs4, ICubeTexture)

CubeTexturePs4::CubeTexturePs4(ContextPs4* context)
:	m_context(context)
{
}

CubeTexturePs4::~CubeTexturePs4()
{
	destroy();
}

bool CubeTexturePs4::create(const CubeTextureCreateDesc& desc)
{
	return true;
}

void CubeTexturePs4::destroy()
{
}

ITexture* CubeTexturePs4::resolve()
{
	return this;
}

int32_t CubeTexturePs4::getMips() const
{
	return 0;
}

int32_t CubeTexturePs4::getSide() const
{
	return 0;
}

bool CubeTexturePs4::lock(int32_t side, int32_t level, Lock& lock)
{
	return true;
}

void CubeTexturePs4::unlock(int32_t side, int32_t level)
{
}

	}
}
