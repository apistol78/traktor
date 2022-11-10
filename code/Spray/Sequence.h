/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityEvent;

	}

	namespace spray
	{

class SequenceInstance;

/*! Trigger sequence.
 * \ingroup Spray
 */
class T_DLLCLASS Sequence : public Object
{
	T_RTTI_CLASS;

public:
	struct Key
	{
		float T;
		Ref< world::IEntityEvent > event;
	};

	explicit Sequence(const AlignedVector< Key >& keys);

	Ref< SequenceInstance > createInstance() const;

private:
	friend class SequenceInstance;

	AlignedVector< Key > m_keys;
};

	}
}

