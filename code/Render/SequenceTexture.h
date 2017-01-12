#ifndef traktor_render_SequenceTexture_H
#define traktor_render_SequenceTexture_H

#include <vector>
#include "Core/Timer/Timer.h"
#include "Render/ITexture.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{
	
class T_DLLCLASS SequenceTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

private:
	friend class SequenceTextureFactory;

	Timer m_time;
	float m_rate;
	std::vector< resource::Proxy< ITexture > > m_textures;
};
	
	}
}

#endif	// traktor_render_SequenceTexture_H
