#ifndef traktor_render_OpenVRCompositor_H
#define traktor_render_OpenVRCompositor_H

#include "Render/IVRCompositor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENVR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace vr
{

class IVRSystem;

}

namespace traktor
{
	namespace render
	{

class RenderTargetSet;

/*! \brief OpenVR compositor implementation.
 * \ingroup Render
 */
class T_DLLCLASS OpenVRCompositor : public IVRCompositor
{
	T_RTTI_CLASS;
	
public:
	OpenVRCompositor();

	virtual bool create(IRenderSystem* renderSystem, IRenderView* renderView) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int32_t getWidth() const T_OVERRIDE T_FINAL;

	virtual int32_t getHeight() const T_OVERRIDE T_FINAL;

	virtual bool beginRenderEye(IRenderView* renderView, int32_t eye) T_OVERRIDE T_FINAL;

	virtual bool endRenderEye(IRenderView* renderView, int32_t eye) T_OVERRIDE T_FINAL;

	virtual bool presentCompositeOutput(IRenderView* renderView) T_OVERRIDE T_FINAL;

private:
	vr::IVRSystem* m_vr;
	Ref< RenderTargetSet > m_targetSet;
};

	}
}

#endif	// traktor_render_OpenVRCompositor_H
