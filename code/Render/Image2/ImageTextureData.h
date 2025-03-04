/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class ImageTexture;
class ITexture;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageTextureData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< const ImageTexture > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ImageGraphPipeline;

	std::wstring m_textureId;
    resource::Id< ITexture > m_texture;
};

}
