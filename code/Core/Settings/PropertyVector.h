/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Vector property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyVector : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Vector4 value_type_t;

	PropertyVector() = default;

	explicit PropertyVector(const value_type_t& value);

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) override final;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

private:
	value_type_t m_value;
};

/*!
 * \ingroup Core
 */
template <>
struct PropertyTrait< Vector4 >
{
	typedef PropertyVector property_type_t;
	typedef const Vector4& default_value_type_t;
	typedef Vector4 return_type_t;
};

}
