/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageTexture : public Object
{
	T_RTTI_CLASS;

public:
	ImageTexture(handle_t textureId, const resource::Proxy< ITexture >& texture);

	handle_t getTextureId() const;

	const resource::Proxy< ITexture >& getTexture() const;

private:
	handle_t m_textureId;
	resource::Proxy< ITexture > m_texture;
};

}
