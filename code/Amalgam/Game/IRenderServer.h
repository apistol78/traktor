#ifndef traktor_amalgam_IRenderServer_H
#define traktor_amalgam_IRenderServer_H

#include "Render/Types.h"
#include "Amalgam/Game/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class IVRCompositor;

	}

	namespace amalgam
	{

/*! \brief Render server.
 * \ingroup Amalgam
 *
 * "Render.Type"				- Render system type.
 * "Render.DepthBits"			- Depth buffer bits.
 * "Render.StencilBits"			- Stencil buffer bits.
 * "Render.MultiSample"			- Multisample taps.
 * "Render.WaitVBlanks"			- Wait for # vblank.
 * "Render.FullScreen"			- Fullscreen mode.
 * "Render.DisplayMode/Width"	- Display width.
 * "Render.DisplayMode/Height"	- Display height.
 * "Render.SkipMips"			- Skip number of mips.
 */
class T_DLLCLASS IRenderServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual render::IRenderSystem* getRenderSystem() = 0;

	virtual render::IRenderView* getRenderView() = 0;

	virtual render::IVRCompositor* getVRCompositor() = 0;

	virtual int32_t getWidth() const = 0;

	virtual int32_t getHeight() const = 0;

	virtual float getScreenAspectRatio() const = 0;

	virtual float getViewAspectRatio() const = 0;

	virtual float getAspectRatio() const = 0;

	virtual bool getStereoscopic() const = 0;

	virtual int32_t getMultiSample() const = 0;

	virtual int32_t getFrameRate() const = 0;

	virtual int32_t getThreadFrameQueueCount() const = 0;
};

	}
}

#endif	// traktor_amalgam_IRenderServer_H
