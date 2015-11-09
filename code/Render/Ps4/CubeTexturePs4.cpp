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

int CubeTexturePs4::getWidth() const
{
	return 0;
}

int CubeTexturePs4::getHeight() const
{
	return 0;
}

int CubeTexturePs4::getDepth() const
{
	return 0;
}

bool CubeTexturePs4::lock(int side, int level, Lock& lock)
{
	return true;
}

void CubeTexturePs4::unlock(int side, int level)
{
}

	}
}
