/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderView;
class ITexture;

class T_DLLCLASS IRenderPlugin : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! Issue plugin's rendering.
	 * 
	 * \note
	 * Currently tailored exclusively for custom upscalers thus
	 * the current arguments.
	 * Will need to be extended in the future if other
	 * kinds of plugins are implemented.
	 */
	virtual void render(IRenderView* renderView, ITexture* colorTexture, ITexture* velocityTexture, ITexture* outputTexture, const Vector4& jitter) = 0;
};

}
