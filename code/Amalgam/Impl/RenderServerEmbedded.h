#pragma once

#include "Amalgam/Impl/RenderServer.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class RenderServerEmbedded : public RenderServer
{
	T_RTTI_CLASS;

public:
	RenderServerEmbedded();

	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp, const SystemWindow& syswin);

	virtual void destroy() override final;

	virtual void createResourceFactories(IEnvironment* environment) override final;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) override final;

	virtual UpdateResult update(PropertyGroup* settings) override final;

	virtual render::IRenderSystem* getRenderSystem() override final;

	virtual render::IRenderView* getRenderView() override final;

	virtual render::IVRCompositor* getVRCompositor() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual float getScreenAspectRatio() const override final;

	virtual float getViewAspectRatio() const override final;

	virtual float getAspectRatio() const override final;

	virtual bool getStereoscopic() const override final;

	virtual int32_t getMultiSample() const override final;

private:
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewEmbeddedDesc m_renderViewDesc;
	float m_screenAspectRatio;
};

	}
}

