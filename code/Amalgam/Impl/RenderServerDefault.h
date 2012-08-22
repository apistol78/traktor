#ifndef traktor_amalgam_RenderServerDefault_H
#define traktor_amalgam_RenderServerDefault_H

#include "Amalgam/Impl/RenderServer.h"

namespace traktor
{
	namespace render
	{
	
class TextureFactory;

	}

	namespace amalgam
	{

class RenderServerDefault : public RenderServer
{
	T_RTTI_CLASS;

public:
	RenderServerDefault();

	bool create(PropertyGroup* settings);

	virtual void destroy();

	virtual void createResourceFactories(IEnvironment* environment);

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings);

	virtual UpdateResult update(PropertyGroup* settings);

	virtual render::IRenderSystem* getRenderSystem();

	virtual render::IRenderView* getRenderView();

	virtual Ref< render::RenderTargetSet > createOffscreenTarget(render::TextureFormat format, bool createDepthStencil, bool usingPrimaryDepthStencil);

	virtual float getScreenAspectRatio() const;

	virtual float getViewAspectRatio() const;

	virtual float getAspectRatio() const;

	virtual bool getStereoscopic() const;

	virtual int32_t getMultiSample() const;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::TextureFactory > m_textureFactory;
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewDefaultDesc m_renderViewDesc;
	float m_screenAspectRatio;
};

	}
}

#endif	// traktor_amalgam_RenderServerDefault_H
