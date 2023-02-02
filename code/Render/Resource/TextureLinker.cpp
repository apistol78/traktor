/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/IProgram.h"
#include "Render/Resource/TextureLinker.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureLinker", TextureLinker, Object)

TextureLinker::TextureLinker(const TextureReader& textureReader)
:	m_textureReader(textureReader)
{
}

bool TextureLinker::link(const ShaderResource::Combination& shaderCombination, IProgram* program) const
{
	const auto& textures = shaderCombination.textures;
	for (uint32_t i = 0; i < textures.size(); ++i)
	{
		Ref< ITexture > texture = m_textureReader.read(textures[i]);
		if (!texture)
			return false;

		const handle_t parameterHandle = getParameterHandleFromTextureReferenceIndex(i);
		program->setTextureParameter(parameterHandle, texture);
	}
	return true;
}

}
