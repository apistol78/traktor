/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Editor/DictionaryAsset.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::i18n
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.i18n.DictionaryAsset", 0, DictionaryAsset, editor::Asset)

DictionaryAsset::DictionaryAsset()
:	m_keyColumn(0)
,	m_textColumn(1)
{
}

void DictionaryAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< int32_t >(L"keyColumn", m_keyColumn);
	s >> Member< int32_t >(L"textColumn", m_textColumn);
}

}
