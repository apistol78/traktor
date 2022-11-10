/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <limits>
#include "Net/Replication/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

enum FloatTemplatePrecision
{
	Ftp32,
	Ftp16,
	Ftp8,
	Ftp4
};

class T_DLLCLASS FloatTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	explicit FloatTemplate(const wchar_t* const tag, float threshold = std::numeric_limits< float >::max());

	explicit FloatTemplate(const wchar_t* const tag, float threshold, float min, float max, FloatTemplatePrecision precision, bool cyclic);

	virtual const TypeInfo& getValueType() const override final;

	virtual uint32_t getMaxPackedDataSize() const override final;

	virtual void pack(BitWriter& writer, const IValue* V) const override final;

	virtual Ref< const IValue > unpack(BitReader& reader) const override final;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const override final;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const override final;

private:
	const wchar_t* const m_tag;
	float m_threshold;
	float m_min;
	float m_max;
	FloatTemplatePrecision m_precision;
	bool m_cyclic;
};

	}
}

