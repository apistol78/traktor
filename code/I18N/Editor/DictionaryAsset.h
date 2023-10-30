/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::i18n
{

class T_DLLCLASS DictionaryAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	DictionaryAsset();

	virtual void serialize(ISerializer& s) override final;

	int32_t getKeyColumn() const { return m_keyColumn; }

	int32_t getTextColumn() const { return m_textColumn; }

private:
	int32_t m_keyColumn;
	int32_t m_textColumn;
};

}
