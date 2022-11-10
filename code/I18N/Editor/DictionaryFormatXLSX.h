/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor
{
	namespace i18n
	{

class DictionaryFormatXLSX : public IDictionaryFormat
{
	T_RTTI_CLASS;

public:
	virtual bool supportExtension(const std::wstring& extension) const override final;

	virtual Ref< Dictionary > read(IStream* stream, int32_t keyColumn, int32_t textColumn) const override final;

	virtual bool write(IStream* stream, const Dictionary* dictionary) const override final;
};

	}
}

