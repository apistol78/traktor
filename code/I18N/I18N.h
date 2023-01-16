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
#include "Core/Ref.h"
#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::i18n
{

class Dictionary;

/*! Internationalization manager.
 * \ingroup I18N
 */
class T_DLLCLASS I18N : public ISingleton
{
public:
	static I18N& getInstance();

	void appendDictionary(const Dictionary* dictionary, bool overrideExisting = false);

	Ref< Dictionary > getMasterDictionary();

	std::wstring get(const std::wstring& id, const std::wstring& defaultText = L"") const;

protected:
	virtual void destroy();

private:
	Ref< Dictionary > m_dictionary;

	I18N();
};

}
