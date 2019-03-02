#pragma once

#include "Render/IVRCompositor.h"
#include "Resource/Proxy.h"

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

	virtual bool create(IRenderSystem* renderSystem, IRenderView* renderView) override final;

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual Matrix44 getProjection(int32_t eye, float nearZ, float farZ) const override final;

	virtual Matrix44 getEyeToHead(int32_t eye) const override final;

	virtual bool beginRenderEye(IRenderView* renderView, int32_t eye) override final;

	virtual bool endRenderEye(IRenderView* renderView, int32_t eye) override final;

	virtual bool presentCompositeOutput(IRenderView* renderView) override final;

private:
	vr::IVRSystem* m_vr;
	Ref< RenderTargetSet > m_targetSet;
};

	}
}

