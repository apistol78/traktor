#ifndef traktor_world_PostProcessDefineTarget_H
#define traktor_world_PostProcessDefineTarget_H

#include "World/PostProcess/PostProcessDefine.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Define post processing render target.
 * \ingroup World
 */
class T_DLLCLASS PostProcessDefineTarget : public PostProcessDefine
{
	T_RTTI_CLASS;

public:
	PostProcessDefineTarget();

	virtual bool define(PostProcess* postProcess, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight);

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_screenDenom;
	render::TextureFormat m_format;
	bool m_depthStencil;
	int32_t m_multiSample;
};

	}
}

#endif	// traktor_world_PostProcessDefineTarget_H
