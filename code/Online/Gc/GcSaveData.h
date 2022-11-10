/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class GcSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	GcSaveData();

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) override final;

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment) override final;

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) override final;

	virtual bool remove(const std::wstring& saveDataId) override final;
};

	}
}

