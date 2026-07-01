/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/RenderPluginVrfy.h"

#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/TextureVrfy.h"

namespace traktor::render
{
	namespace
	{
	
ITexture* unwrapTexture(ITexture* texture)
{
	TextureVrfy* tv = dynamic_type_cast< TextureVrfy* >(texture);
	return (tv != nullptr) ? tv->getTexture() : nullptr;
}
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPluginVrfy", RenderPluginVrfy, IRenderPlugin)

RenderPluginVrfy::RenderPluginVrfy(IRenderPlugin* wrappedPlugin)
	: m_wrappedPlugin(wrappedPlugin)
{
}

void RenderPluginVrfy::destroy()
{
	safeDestroy(m_wrappedPlugin);
}

void RenderPluginVrfy::render(IRenderView* renderView, ITexture* colorTexture, ITexture* depthTexture, ITexture* velocityTexture, ITexture* outputTexture, const Vector4& jitter)
{
	IRenderView* rv = mandatory_non_null_type_cast< RenderViewVrfy* >(renderView)->getRenderView();
	ITexture* ct = unwrapTexture(colorTexture);
	ITexture* dt = unwrapTexture(depthTexture);
	ITexture* vt = unwrapTexture(velocityTexture);
	ITexture* ot = unwrapTexture(outputTexture);

	if (m_wrappedPlugin)
		m_wrappedPlugin->render(rv, ct, dt, vt, ot, jitter);
}

}
