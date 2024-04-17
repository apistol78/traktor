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
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

class Act;
class TrackData;

/*! Act description.
 * \ingroup Theater
 */
class T_DLLCLASS ActData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Act > createInstance(float start, float end) const;

	virtual void serialize(ISerializer& s) override final;

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	const RefArray< TrackData >& getTracks() const { return m_tracks; }

	RefArray< TrackData >& getTracks() { return m_tracks; }

private:
	std::wstring m_name;
	float m_duration = 5.0f;
	RefArray< TrackData > m_tracks;
};

}
