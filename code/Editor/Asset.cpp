/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Asset.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Asset", Asset, ISerializable)

Asset::Asset(const Path& fileName)
:	m_fileName(fileName)
{
}

void Asset::setFileName(const Path& fileName)
{
	m_fileName = fileName;
}

const Path& Asset::getFileName() const
{
	return m_fileName;
}

void Asset::serialize(ISerializer& s)
{
	s >> Member< Path >(L"fileName", m_fileName);
}

}
