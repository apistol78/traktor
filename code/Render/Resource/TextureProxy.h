#ifndef traktor_render_TextureProxy_H
#define traktor_render_TextureProxy_H

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
	TextureProxy(const resource::Proxy< ITexture >& texture);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

private:
	resource::Proxy< ITexture > m_texture;
};

	}
}

#endif	// traktor_render_TextureProxy_H
