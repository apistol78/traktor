#ifndef traktor_amalgam_RenderServerEmbedded_H
#define traktor_amalgam_RenderServerEmbedded_H

#include "Amalgam/Impl/RenderServer.h"

namespace traktor
{
	namespace amalgam
	{

class RenderServerEmbedded : public RenderServer
{
	T_RTTI_CLASS;

public:
	RenderServerEmbedded(net::BidirectionalObjectTransport* transport);

	bool create(PropertyGroup* settings, void* nativeHandle, void* nativeWindowHandle);

	virtual void destroy();

	virtual void createResourceFactories(IEnvironment* environment);

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings);

	virtual UpdateResult update(PropertyGroup* settings);

	virtual render::IRenderSystem* getRenderSystem();

	virtual render::IRenderView* getRenderView();

	virtual float getScreenAspectRatio() const;

	virtual float getViewAspectRatio() const;

	virtual float getAspectRatio() const;

	virtual bool getStereoscopic() const;

	virtual int32_t getMultiSample() const;

private:
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewEmbeddedDesc m_renderViewDesc;
	float m_screenAspectRatio;
};

	}
}

#endif	// traktor_amalgam_RenderServerEmbedded_H
