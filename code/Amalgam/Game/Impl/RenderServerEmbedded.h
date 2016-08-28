#ifndef traktor_amalgam_RenderServerEmbedded_H
#define traktor_amalgam_RenderServerEmbedded_H

#include "Amalgam/Game/Impl/RenderServer.h"

namespace traktor
{
	namespace amalgam
	{

class RenderServerEmbedded : public RenderServer
{
	T_RTTI_CLASS;

public:
	RenderServerEmbedded();

	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp, const SystemWindow& syswin);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void createResourceFactories(IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) T_OVERRIDE T_FINAL;

	virtual UpdateResult update(PropertyGroup* settings) T_OVERRIDE T_FINAL;

	virtual render::IRenderSystem* getRenderSystem() T_OVERRIDE T_FINAL;

	virtual render::IRenderView* getRenderView() T_OVERRIDE T_FINAL;

	virtual render::IVRCompositor* getVRCompositor() T_OVERRIDE T_FINAL;

	virtual int32_t getWidth() const T_OVERRIDE T_FINAL;

	virtual int32_t getHeight() const T_OVERRIDE T_FINAL;

	virtual float getScreenAspectRatio() const T_OVERRIDE T_FINAL;

	virtual float getViewAspectRatio() const T_OVERRIDE T_FINAL;

	virtual float getAspectRatio() const T_OVERRIDE T_FINAL;

	virtual bool getStereoscopic() const T_OVERRIDE T_FINAL;

	virtual int32_t getMultiSample() const T_OVERRIDE T_FINAL;

private:
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewEmbeddedDesc m_renderViewDesc;
	float m_screenAspectRatio;
};

	}
}

#endif	// traktor_amalgam_RenderServerEmbedded_H
