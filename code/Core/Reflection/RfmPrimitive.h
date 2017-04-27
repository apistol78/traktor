/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_RfmPrimitive_H
#define traktor_RfmPrimitive_H

#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Primitive member reflection.
 * \ingroup Core
 */
template < typename PrimitiveType >
class RfmPrimitive : public ReflectionMember
{
public:
	typedef PrimitiveType type_t;

	void set(const type_t& value) { m_value = value; }

	const type_t& get() const { return m_value; }

	virtual bool replace(const ReflectionMember* source)
	{
		typedef RfmPrimitive< PrimitiveType > class_type_t;
		if (const class_type_t* sourceType = dynamic_type_cast< const class_type_t* >(source))
		{
			m_value = sourceType->m_value;
			return true;
		}
		else
			return false;
	}

protected:
	RfmPrimitive(const wchar_t* name, const type_t& value)
	:	ReflectionMember(name)
	,	m_value(value)
	{
	}

private:
	type_t T_ALIGN16 m_value;
};

#define T_DEFINE_DERIVED_CLASS(NAME, TYPE, TYPE_CONST_REF)	\
	class T_DLLCLASS NAME : public RfmPrimitive< TYPE >		\
	{														\
		T_RTTI_CLASS;										\
															\
	public:													\
		NAME(const wchar_t* name, TYPE_CONST_REF value)		\
		:	RfmPrimitive< TYPE >(name, value)				\
		{													\
		}													\
	};														\

T_DEFINE_DERIVED_CLASS(RfmPrimitiveBoolean, bool, bool)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveInt8, int8_t, int8_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveUInt8, uint8_t, uint8_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveInt16, int16_t, int16_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveUInt16, uint16_t, uint16_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveInt32, int32_t, int32_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveUInt32, uint32_t, uint32_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveInt64, int64_t, int64_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveUInt64, uint64_t, uint64_t)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveFloat, float, float)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveDouble, double, double)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveString, std::string, const std::string&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveWideString, std::wstring, const std::wstring&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveGuid, Guid, const Guid&)
T_DEFINE_DERIVED_CLASS(RfmPrimitivePath, Path, const Path&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveColor4ub, Color4ub, const Color4ub&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveColor4f, Color4f, const Color4f&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveScalar, Scalar, const Scalar&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveVector2, Vector2, const Vector2&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveVector4, Vector4, const Vector4&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveMatrix33, Matrix33, const Matrix33&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveMatrix44, Matrix44, const Matrix44&)
T_DEFINE_DERIVED_CLASS(RfmPrimitiveQuaternion, Quaternion, const Quaternion&)

#undef T_DEFINE_DERIVED_CLASS

}

#endif	// traktor_RfmPrimitive_H
