/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/SimpleTexturePs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTexturePs4", SimpleTexturePs4, ISimpleTexture)

SimpleTexturePs4::SimpleTexturePs4(ContextPs4* context)
:	m_context(context)
{
}

SimpleTexturePs4::~SimpleTexturePs4()
{
	destroy();
}

bool SimpleTexturePs4::create(const SimpleTextureCreateDesc& desc)
{
	return true;
}

void SimpleTexturePs4::destroy()
{
}

ITexture* SimpleTexturePs4::resolve()
{
	return this;
}

int SimpleTexturePs4::getWidth() const
{
	return 0;
}

int SimpleTexturePs4::getHeight() const
{
	return 0;
}

bool SimpleTexturePs4::lock(int level, Lock& lock)
{
	return true;
}

void SimpleTexturePs4::unlock(int level)
{
}

void* SimpleTexturePs4::getInternalHandle()
{
	return 0;
}

	}
}
