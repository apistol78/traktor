/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sb
{

class T_DLLCLASS ProjectItem : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const RefArray< ProjectItem >& getItems() const;

	virtual void serialize(ISerializer& s) override;

private:
	RefArray< ProjectItem > m_items;
};

}
