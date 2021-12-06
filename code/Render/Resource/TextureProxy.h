#pragma once

#include "Render/ITexture.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace render
	{

class TextureProxy : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit TextureProxy(const resource::Proxy< ITexture >& texture);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

private:
	resource::Proxy< ITexture > m_texture;
};

	}
}
