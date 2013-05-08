#ifndef traktor_world_PostProcessDefineTarget_H
#define traktor_world_PostProcessDefineTarget_H

#include "Core/Math/Color4f.h"
#include "Render/Types.h"
#include "World/PostProcess/PostProcessDefine.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

	virtual void serialize(ISerializer& s);

	const std::wstring& getId() const { return m_id; }

private:
	std::wstring m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_screenWidthDenom;
	uint32_t m_screenHeightDenom;
	uint32_t m_maxWidth;
	uint32_t m_maxHeight;
	render::TextureFormat m_format;
	bool m_depthStencil;
	bool m_preferTiled;
	int32_t m_multiSample;
	Color4f m_clearColor;
};

	}
}

#endif	// traktor_world_PostProcessDefineTarget_H
