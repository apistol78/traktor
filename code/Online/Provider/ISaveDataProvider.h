/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Online/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace online
	{

class T_DLLCLASS ISaveDataProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) = 0;

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment) = 0;

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) = 0;

	virtual bool remove(const std::wstring& saveDataId) = 0;
};

	}
}

