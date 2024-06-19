/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "World/IWorldComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

class ActData;
class TheaterComponent;

/*! Theater world component data.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterComponentData : public world::IWorldComponentData
{
	T_RTTI_CLASS;

public:
	Ref< TheaterComponent > createInstance(bool editor) const;

	virtual void serialize(ISerializer& s) override final;

	const RefArray< ActData >& getActs() const { return m_acts; }

	RefArray< ActData >& getActs() { return m_acts; }

	float getActStartTime(int32_t act) const;

private:
	friend class TheaterComponentPipeline;

	RefArray< ActData > m_acts;
	bool m_repeatActs = false;
	bool m_randomizeActs = false;
};

}
