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
#include "Core/Math/Transform.h"
#include "Spray/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

struct Context;

/*! Sequence instance.
 * \ingroup Spray
 */
class T_DLLCLASS SequenceInstance : public Object
{
	T_RTTI_CLASS;

public:
	void update(Context& context, const Transform& transform, float T, bool enable);

private:
	friend class Sequence;

	Ref< const Sequence > m_sequence;
	int32_t m_index;

	explicit SequenceInstance(const Sequence* sequence);
};

	}
}

