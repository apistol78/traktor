/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <xess/xess_vk.h>

#include "Render/IRenderPlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS RenderPluginXeSS : public IRenderPlugin
{
	T_RTTI_CLASS;

public:
	static void getExtensions(AlignedVector< const char* >& outExtensions);

	static void getDeviceExtensions(VkInstance instance, VkPhysicalDevice physicalDevice, AlignedVector< const char* >& outExtensions);

	virtual void destroy() override final;

	virtual void render(IRenderView* renderView, ITexture* colorTexture, ITexture* velocityTexture, ITexture* outputTexture, const Vector4& jitter) override final;

private:
	xess_context_handle_t m_xessContext = nullptr;

	int32_t m_initWidth = -1;
	int32_t m_initHeight = -1;
};

}
