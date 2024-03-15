/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"
#include "Runtime/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;
class IRenderView;

}

namespace traktor::runtime
{

/*! Render server.
 * \ingroup Runtime
 *
 * "Render.Type"				- Render system type.
 * "Render.DepthBits"			- Depth buffer bits.
 * "Render.StencilBits"			- Stencil buffer bits.
 * "Render.MultiSample"			- Multisample taps.
 * "Render.WaitVBlanks"			- Wait for # vblank.
 * "Render.FullScreen"			- Fullscreen mode.
 * "Render.DisplayMode/Width"	- Display width.
 * "Render.DisplayMode/Height"	- Display height.
 * "Render.SkipMips"			- Skip number of mips.
 */
class T_DLLCLASS IRenderServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual render::IRenderSystem* getRenderSystem() = 0;

	virtual render::IRenderView* getRenderView() = 0;

	virtual int32_t getWidth() const = 0;

	virtual int32_t getHeight() const = 0;

	virtual float getScreenAspectRatio() const = 0;

	virtual float getViewAspectRatio() const = 0;

	virtual float getAspectRatio() const = 0;

	virtual int32_t getMultiSample() const = 0;

	virtual float getRefreshRate() const = 0;

	virtual double getCPUDuration() const = 0;

	virtual double getGPUDuration() const = 0;

	virtual int32_t getFrameRate() const = 0;

	virtual int32_t getThreadFrameQueueCount() const = 0;
};

}
