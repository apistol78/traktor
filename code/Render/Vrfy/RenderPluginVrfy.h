/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/IRenderPlugin.h"

namespace traktor::render
{

/*!
 * \ingroup Vrfy
 */
class RenderPluginVrfy : public IRenderPlugin
{
	T_RTTI_CLASS;

public:
	explicit RenderPluginVrfy(IRenderPlugin* wrappedPlugin);

	virtual void destroy() override final;

	virtual void render(IRenderView* renderView, ITexture* colorTexture, ITexture* velocityTexture, ITexture* outputTexture, const Vector4& jitter) override final;

private:
	Ref< IRenderPlugin > m_wrappedPlugin;
};

}
