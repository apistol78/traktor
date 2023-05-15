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
#include "Jungle/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::jungle
{

class T_DLLCLASS BooleanTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	explicit BooleanTemplate(const std::wstring& tag, float threshold);

	virtual const TypeInfo& getValueType() const override final;

	virtual uint32_t getMaxPackedDataSize() const override final;

	virtual void pack(BitWriter& writer, const IValue* V) const override final;

	virtual Ref< const IValue > unpack(BitReader& reader) const override final;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const override final;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const override final;

private:
	std::wstring m_tag;
	float m_threshold;
};

}
