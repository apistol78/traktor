#ifndef traktor_amalgam_RenderServerDefault_H
#define traktor_amalgam_RenderServerDefault_H

#include "Amalgam/Game/Impl/RenderServer.h"

namespace traktor
{
	namespace amalgam
	{

class RenderServerDefault : public RenderServer
{
	T_RTTI_CLASS;

public:
	RenderServerDefault();

	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, void* nativeHandle);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void createResourceFactories(IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) T_OVERRIDE T_FINAL;

	virtual UpdateResult update(PropertyGroup* settings) T_OVERRIDE T_FINAL;

	virtual render::IRenderSystem* getRenderSystem() T_OVERRIDE T_FINAL;

	virtual render::IRenderView* getRenderView() T_OVERRIDE T_FINAL;

	virtual float getScreenAspectRatio() const T_OVERRIDE T_FINAL;

	virtual float getViewAspectRatio() const T_OVERRIDE T_FINAL;

	virtual float getAspectRatio() const T_OVERRIDE T_FINAL;

	virtual bool getStereoscopic() const T_OVERRIDE T_FINAL;

	virtual bool getVR() const T_OVERRIDE T_FINAL;

	virtual int32_t getMultiSample() const T_OVERRIDE T_FINAL;

private:
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewDefaultDesc m_renderViewDesc;
	float m_screenAspectRatio;
	bool m_vr;
};

	}
}

#endif	// traktor_amalgam_RenderServerDefault_H
