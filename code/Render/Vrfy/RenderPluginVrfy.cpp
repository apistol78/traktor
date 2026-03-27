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
	ITexture* ct = mandatory_non_null_type_cast< TextureVrfy* >(colorTexture)->getTexture();
	ITexture* dt = mandatory_non_null_type_cast< TextureVrfy* >(depthTexture)->getTexture();
	ITexture* vt = mandatory_non_null_type_cast< TextureVrfy* >(velocityTexture)->getTexture();
	ITexture* ot = mandatory_non_null_type_cast< TextureVrfy* >(outputTexture)->getTexture();

	if (m_wrappedPlugin)
		m_wrappedPlugin->render(rv, ct, dt, vt, ot, jitter);
}

}
