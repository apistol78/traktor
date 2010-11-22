#ifndef traktor_amalgam_RenderServer_H
#define traktor_amalgam_RenderServer_H

#include "Amalgam/IRenderServer.h"

namespace traktor
{

class Settings;

	namespace render
	{

class TextureFactory;

	}

	namespace amalgam
	{

class Configuration;
class IEnvironment;

class RenderServer : public IRenderServer
{
	T_RTTI_CLASS;

public:
	RenderServer();

	bool create(Settings* settings);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	void update(Settings* settings);

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

#endif	// traktor_amalgam_RenderServer_H
