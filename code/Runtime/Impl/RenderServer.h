/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IRenderServer.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::render
{

class TextureFactory;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class RenderServer : public IRenderServer
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrTerminate = 0,
		UrSuccess = 1,
		UrReconfigure = 2
	};

	virtual void destroy() = 0;

	virtual void createResourceFactories(IEnvironment* environment) = 0;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) = 0;

	virtual UpdateResult update(PropertyGroup* settings);

	virtual double getCPUDuration() const;

	virtual double getGPUDuration() const;

	virtual int32_t getFrameRate() const override final;

	virtual int32_t getThreadFrameQueueCount() const override final;

	void setDurations(double cpuDuration, double gpuDuration);

	void setFrameRate(int32_t frameRate);

protected:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::TextureFactory > m_textureFactory;

private:
	std::atomic< double > m_cpuDuration = 0.0;
	std::atomic< double > m_gpuDuration = 0.0;
	std::atomic< int32_t > m_frameRate = 0;
};

}
