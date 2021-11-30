#pragma once

#include "Runtime/Impl/RenderServer.h"

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class RenderServerDefault : public RenderServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp);

	virtual void destroy() override final;

	virtual void createResourceFactories(IEnvironment* environment) override final;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) override final;

	virtual UpdateResult update(PropertyGroup* settings) override final;

	virtual render::IRenderSystem* getRenderSystem() override final;

	virtual render::IRenderView* getRenderView() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual float getScreenAspectRatio() const override final;

	virtual float getViewAspectRatio() const override final;

	virtual float getAspectRatio() const override final;

	virtual int32_t getMultiSample() const override final;

private:
	render::DisplayMode m_originalDisplayMode;
	render::RenderViewDefaultDesc m_renderViewDesc;
	float m_screenAspectRatio = 1.0f;
};

	}
}

