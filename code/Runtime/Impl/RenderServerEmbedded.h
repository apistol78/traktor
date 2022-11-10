/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Impl/RenderServer.h"

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class RenderServerEmbedded : public RenderServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp, const SystemWindow& syswin);

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
	render::RenderViewEmbeddedDesc m_renderViewDesc;
	float m_screenAspectRatio = 1.0f;
};

	}
}

