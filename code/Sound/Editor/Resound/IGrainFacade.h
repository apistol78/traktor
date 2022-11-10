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
#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace sound
	{

class IGrainData;

class IGrainFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual ui::Widget* createView(IGrainData* grain, ui::Widget* parent) = 0;

	virtual int32_t getImage(const IGrainData* grain) const = 0;

	virtual std::wstring getText(const IGrainData* grain) const = 0;

	virtual bool getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const = 0;

	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(IGrainData* parentGrain, IGrainData* childGrain) = 0;

	virtual bool removeChild(IGrainData* parentGrain, IGrainData* childGrain) = 0;

	virtual bool getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren) = 0;
};

	}
}

