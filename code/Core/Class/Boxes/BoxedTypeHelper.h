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
#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

// Helper to get type name for boxed inner types (handles both RTTI and primitive types)
template < typename T, bool HasRtti = std::is_base_of< ITypedObject, T >::value >
struct BoxedInnerTypeName
{
	static std::wstring get() { return type_name< T >(); }
};

// Forward declarations for math types
class Matrix33;
class Matrix44;
class Quaternion;
class Vector2;
class Vector4;
class Color4f;
class Color4ub;
class Scalar;
class Any;

// Specializations for primitive types that don't have RTTI
template <> struct BoxedInnerTypeName< float, false > { static std::wstring get() { return L"float"; } };
template <> struct BoxedInnerTypeName< double, false > { static std::wstring get() { return L"double"; } };
template <> struct BoxedInnerTypeName< int32_t, false > { static std::wstring get() { return L"int32"; } };
template <> struct BoxedInnerTypeName< int64_t, false > { static std::wstring get() { return L"int64"; } };
template <> struct BoxedInnerTypeName< uint32_t, false > { static std::wstring get() { return L"uint32"; } };
template <> struct BoxedInnerTypeName< uint64_t, false > { static std::wstring get() { return L"uint64"; } };
template <> struct BoxedInnerTypeName< bool, false > { static std::wstring get() { return L"bool"; } };
template <> struct BoxedInnerTypeName< std::wstring, false > { static std::wstring get() { return L"std::wstring"; } };
template <> struct BoxedInnerTypeName< std::string, false > { static std::wstring get() { return L"std::string"; } };

// Specializations for math types that don't have RTTI
template <> struct BoxedInnerTypeName< Matrix33, false > { static std::wstring get() { return L"traktor.Matrix33"; } };
template <> struct BoxedInnerTypeName< Matrix44, false > { static std::wstring get() { return L"traktor.Matrix44"; } };
template <> struct BoxedInnerTypeName< Quaternion, false > { static std::wstring get() { return L"traktor.Quaternion"; } };
template <> struct BoxedInnerTypeName< Vector2, false > { static std::wstring get() { return L"traktor.Vector2"; } };
template <> struct BoxedInnerTypeName< Vector4, false > { static std::wstring get() { return L"traktor.Vector4"; } };
template <> struct BoxedInnerTypeName< Color4f, false > { static std::wstring get() { return L"traktor.Color4f"; } };
template <> struct BoxedInnerTypeName< Color4ub, false > { static std::wstring get() { return L"traktor.Color4ub"; } };
template <> struct BoxedInnerTypeName< Scalar, false > { static std::wstring get() { return L"traktor.Scalar"; } };
template <> struct BoxedInnerTypeName< Any, false > { static std::wstring get() { return L"traktor.Any"; } };

}
