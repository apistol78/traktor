#ifndef traktor_script_Boxes_H
#define traktor_script_Boxes_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Range.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Script/Any.h"
#include "Script/CastAny.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS Boxed : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring toString() const = 0;
};

class T_DLLCLASS BoxedUInt64 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedUInt64();

	explicit BoxedUInt64(uint64_t value);

	std::wstring format() const;

	uint64_t unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	uint64_t m_value;
};

class T_DLLCLASS BoxedGuid : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedGuid();

	explicit BoxedGuid(const Guid& value);

	explicit BoxedGuid(const std::wstring& value);

	static Guid create() { return Guid::create(); }

	void set(const std::wstring& str) { m_value = Guid(str); }

	std::wstring format() const { return m_value.format(); }

	bool isValid() const { return m_value.isValid(); }

	bool isNull() const { return m_value.isNull(); }

	bool isNotNull() const { return m_value.isNotNull(); }

	const Guid& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Guid m_value;
};

class T_DLLCLASS BoxedVector2 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedVector2();

	explicit BoxedVector2(const Vector2& value);

	explicit BoxedVector2(float x, float y);

	void set(float x, float y) { m_value.set(x, y); }

	float x() const { return m_value.x; }

	float y() const { return m_value.y; }

	Vector2 add(const Vector2& v) const;

	Vector2 sub(const Vector2& v) const;

	Vector2 mul(const Vector2& v) const;

	Vector2 div(const Vector2& v) const;

	Vector2 add(float v) const;

	Vector2 sub(float v) const;

	Vector2 mul(float v) const;

	Vector2 div(float v) const;

	const Vector2& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Vector2 m_value;
};

class T_DLLCLASS BoxedVector4 : public Boxed
{
	T_RTTI_CLASS;
	
public:
	BoxedVector4();
	
	explicit BoxedVector4(const Vector4& value);

	explicit BoxedVector4(float x, float y, float z);
	
	explicit BoxedVector4(float x, float y, float z, float w);

	void set(float x, float y, float z, float w);
	
	float x() const { return m_value.x(); }
	
	float y() const { return m_value.y(); }
	
	float z() const { return m_value.z(); }
	
	float w() const { return m_value.w(); }

	Vector4 xyz0() const { return m_value.xyz0(); }

	Vector4 xyz1() const { return m_value.xyz1(); }

	Vector4 add(const Vector4& v) const;

	Vector4 sub(const Vector4& v) const;

	Vector4 mul(const Vector4& v) const;

	Vector4 div(const Vector4& v) const;

	Vector4 add(float v) const;

	Vector4 sub(float v) const;

	Vector4 mul(float v) const;

	Vector4 div(float v) const;

	float dot(const Vector4& v) const;

	Vector4 cross(const Vector4& v) const;

	float length() const;

	Vector4 normalized() const;

	Vector4 neg() const;

	static Vector4 zero() { return Vector4::zero(); }

	static Vector4 origo() { return Vector4::origo(); }

	static Vector4 lerp(const Vector4& a, const Vector4& b, float c) { return traktor::lerp(a, b, Scalar(c)); }
	
	const Vector4& unbox() const { return m_value; }

	virtual std::wstring toString() const;
	
private:
	Vector4 m_value;
};

class T_DLLCLASS BoxedQuaternion : public Boxed
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const Vector4& axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const Vector4& from, const Vector4& to);
	
	float x() const { return m_value.e.x(); }
	
	float y() const { return m_value.e.y(); }
	
	float z() const { return m_value.e.z(); }
	
	float w() const { return m_value.e.w(); }

	Quaternion normalized() const;

	Quaternion inverse() const;

	Quaternion concat(const Quaternion& q) const;

	Vector4 transform(const Vector4& v) const;

	Vector4 getEulerAngles() const;

	Vector4 getAxisAngle() const;

	static Quaternion identity() { return Quaternion::identity(); }

	static Quaternion fromEulerAngles(float head, float pitch, float bank);

	static Quaternion fromAxisAngle(const Vector4& axisAngle);

	static Quaternion lerp(const Quaternion& a, const Quaternion& b, float c) { return traktor::lerp(a, b, c); }
	
	const Quaternion& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Quaternion m_value;
};

class T_DLLCLASS BoxedPlane : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedPlane();

	explicit BoxedPlane(const Plane& value);

	explicit BoxedPlane(const Vector4& normal, float distance);

	explicit BoxedPlane(const Vector4& normal, const Vector4& pointInPlane);

	explicit BoxedPlane(const Vector4& a, const Vector4& b, const Vector4& c);

	explicit BoxedPlane(float a, float b, float c, float d);

	void setNormal(const Vector4& normal) { m_value.setNormal(normal); }

	void setDistance(float distance) { m_value.setDistance(Scalar(distance)); }

	Vector4 normal() const { return m_value.normal(); }

	float distance() const { return m_value.distance(); }

	float distanceToPoint(const Vector4& point) const { return m_value.distance(point); }

	Vector4 project(const Vector4& v) const { return m_value.project(v); }

	const Plane& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Plane m_value;
};

class T_DLLCLASS BoxedTransform : public Boxed
{
	T_RTTI_CLASS;
	
public:
	BoxedTransform();
	
	explicit BoxedTransform(const Transform& value);
	
	explicit BoxedTransform(const Vector4& translation, const Quaternion& rotation);
	
	const Vector4& translation() const;
	
	const Quaternion& rotation() const;

	Vector4 axisX() const;

	Vector4 axisY() const;

	Vector4 axisZ() const;

	Plane planeX() const;

	Plane planeY() const;

	Plane planeZ() const;

	Transform inverse() const;

	Transform concat(const Transform& t) const;

	Vector4 transform(const Vector4& v) const;
	
	static Transform identity() { return Transform::identity(); }

	static Transform lerp(const Transform& a, const Transform& b, float c) { return traktor::lerp(a, b, Scalar(c)); }

	const Transform& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Transform m_value;
};

class T_DLLCLASS BoxedAabb3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAabb3();

	explicit BoxedAabb3(const Aabb3& value);

	explicit BoxedAabb3(const Vector4& min, const Vector4& max);

	bool inside(const Vector4& pt) const { return m_value.inside(pt); }

	Aabb3 transform(const Transform& tf) const { return m_value.transform(tf); }

	const Vector4& getMin() const { return m_value.mn; }

	const Vector4& getMax() const { return m_value.mx; }

	Vector4 getCenter() const { return m_value.getCenter(); }

	Vector4 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	bool overlap(const Aabb3& aabb) const { return m_value.overlap(aabb); }

	const Aabb3& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Aabb3 m_value;
};

class T_DLLCLASS BoxedColor4f : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedColor4f();

	explicit BoxedColor4f(const Color4f& value);

	explicit BoxedColor4f(float red, float green, float blue);

	explicit BoxedColor4f(float red, float green, float blue, float alpha);

	float get(int32_t channel) const { return m_value.get(channel); }

	float getRed() const { return m_value.getRed(); }

	float getGreen() const { return m_value.getGreen(); }

	float getBlue() const { return m_value.getBlue(); }

	float getAlpha() const { return m_value.getAlpha(); }

	void set(int32_t channel, float value) { m_value.set(channel, Scalar(value)); }

	void setRed(float red) { m_value.setRed(Scalar(red)); }

	void setGreen(float green) { m_value.setGreen(Scalar(green)); }

	void setBlue(float blue) { m_value.setBlue(Scalar(blue)); }

	void setAlpha(float alpha) { m_value.setAlpha(Scalar(alpha)); }

	const Color4f& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Color4f m_value;
};

class T_DLLCLASS BoxedColor4ub : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedColor4ub();

	explicit BoxedColor4ub(const Color4ub& value);

	explicit BoxedColor4ub(uint8_t r, uint8_t g, uint8_t b);

	explicit BoxedColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	uint8_t getRed() const { return m_value.r; }

	uint8_t getGreen() const { return m_value.g; }

	uint8_t getBlue() const { return m_value.b; }

	uint8_t getAlpha() const { return m_value.a; }

	uint32_t getARGB() const { return m_value.getARGB(); }

	void setRed(uint8_t red) { m_value.r = red; }

	void setGreen(uint8_t green) { m_value.g = green; }

	void setBlue(uint8_t blue) { m_value.b = blue; }

	void setAlpha(uint8_t alpha) { m_value.a = alpha; }

	const Color4ub& unbox() const { return m_value; }

	virtual std::wstring toString() const;

private:
	Color4ub m_value;
};

class T_DLLCLASS BoxedRefArray : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRefArray();

	template < typename ObjectType >
	BoxedRefArray(const RefArray< ObjectType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = arr[i];
	}

	int32_t size() const;

	void set(int32_t index, Object* object);

	Object* get(int32_t index);

	void pushBack(Object* object);

	void popBack();

	Object* front();

	Object* back();

	virtual std::wstring toString() const;

	template < typename ObjectType >
	RefArray< ObjectType > unbox() const
	{
		RefArray< ObjectType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = checked_type_cast< ObjectType*, true >(m_arr[i]);
		return arr;
	}

private:
	RefArray< Object > m_arr;
};

class T_DLLCLASS BoxedRange : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRange();

	template < typename ItemType >
	BoxedRange(const Range< ItemType >& range)
	{
		m_min = CastAny< ItemType >::set(range.min);
		m_max = CastAny< ItemType >::set(range.max);
	}

	const Any& min() const { return m_min; }

	const Any& max() const { return m_max; }

	virtual std::wstring toString() const;

	template < typename ItemType >
	Range< ItemType > unbox() const
	{
		return Range< ItemType >(
			CastAny< ItemType >::get(m_min),
			CastAny< ItemType >::get(m_max)
		);
	}

private:
	Any m_min;
	Any m_max;
};

class T_DLLCLASS BoxedStdVector : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedStdVector();

	template < typename ItemType >
	BoxedStdVector(const std::vector< ItemType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = CastAny< ItemType >::set(arr[i]);
	}

	int32_t size() const;

	void set(int32_t index, const Any& value);

	Any get(int32_t index);

	virtual std::wstring toString() const;

	template < typename ItemType >
	std::vector< ItemType > unbox() const
	{
		std::vector< ItemType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = CastAny< ItemType >::get(m_arr[i]);
		return arr;
	}

private:
	std::vector< Any > m_arr;
};

template < >
struct CastAny< uint64_t, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedUInt64 >(value.getObjectUnsafe());
	}
	static Any set(const uint64_t& value) {
		return Any::fromObject(new BoxedUInt64(value));
	}	
	static uint64_t get(const Any& value) {
		return checked_type_cast< BoxedUInt64*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny< const uint64_t&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedUInt64 >(value.getObjectUnsafe());
	}
	static Any set(const uint64_t& value) {
		return Any::fromObject(new BoxedUInt64(value));
	}	
	static uint64_t get(const Any& value) {
		return checked_type_cast< BoxedUInt64*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Guid, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}	
	static Guid get(const Any& value) {
		return checked_type_cast< BoxedGuid*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Guid&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}	
	static Guid get(const Any& value) {
		return checked_type_cast< BoxedGuid*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Vector2, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector2 >(value.getObjectUnsafe());
	}
	static Any set(const Vector2& value) {
		return Any::fromObject(new BoxedVector2(value));
	}	
	static Vector2 get(const Any& value) {
		return checked_type_cast< BoxedVector2*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Vector2&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector2 >(value.getObjectUnsafe());
	}
	static Any set(const Vector2& value) {
		return Any::fromObject(new BoxedVector2(value));
	}	
	static Vector2 get(const Any& value) {
		return checked_type_cast< BoxedVector2*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Vector4, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4 >(value.getObjectUnsafe());
	}
    static Any set(const Vector4& value) {
        return Any::fromObject(new BoxedVector4(value));
    }	
    static Vector4 get(const Any& value) {
        return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Vector4&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4 >(value.getObjectUnsafe());
	}
    static Any set(const Vector4& value) {
        return Any::fromObject(new BoxedVector4(value));
    }	
    static Vector4 get(const Any& value) {
        return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Quaternion, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static Quaternion get(const Any& value) {
        return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Quaternion&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static Quaternion get(const Any& value) {
        return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Plane, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static Plane get(const Any& value) {
		return checked_type_cast< BoxedPlane*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Plane&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static Plane get(const Any& value) {
		return checked_type_cast< BoxedPlane*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Transform, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTransform >(value.getObjectUnsafe());
	}
    static Any set(const Transform& value) {
        return Any::fromObject(new BoxedTransform(value));
    }
    static Transform get(const Any& value) {
        return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Transform&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTransform >(value.getObjectUnsafe());
	}
    static Any set(const Transform& value) {
        return Any::fromObject(new BoxedTransform(value));
    }
    static Transform get(const Any& value) {
        return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Aabb3, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}	
	static Aabb3 get(const Any& value) {
		return checked_type_cast< BoxedAabb3*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Aabb3&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}	
	static Aabb3 get(const Any& value) {
		return checked_type_cast< BoxedAabb3*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Color4f, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4f >(value.getObjectUnsafe());
	}
	static Any set(const Color4f& value) {
		return Any::fromObject(new BoxedColor4f(value));
	}
	static Color4f get(const Any& value) {
		return checked_type_cast< BoxedColor4f*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Color4f&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4f >(value.getObjectUnsafe());
	}
	static Any set(const Color4f& value) {
		return Any::fromObject(new BoxedColor4f(value));
	}
	static Color4f get(const Any& value) {
		return checked_type_cast< BoxedColor4f*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Color4ub, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static Color4ub get(const Any& value) {
		return checked_type_cast< BoxedColor4ub*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Color4ub&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static Color4ub get(const Any& value) {
		return checked_type_cast< BoxedColor4ub*, false >(value.getObject())->unbox();
	}
};

template < typename InnerType >
struct CastAny< Range< InnerType >, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return checked_type_cast< BoxedRange*, false >(value.getObject())->unbox< InnerType >();
	}
};

template < typename InnerType >
struct CastAny< const Range< InnerType >&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return checked_type_cast< BoxedRange*, false >(value.getObject())->unbox< InnerType >();
	}
};

template < typename InnerType >
struct CastAny < RefArray< InnerType >, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedRefArray*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const RefArray< InnerType >&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedRefArray*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < std::vector< InnerType >, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedStdVector >(value.getObjectUnsafe());
	}
    static Any set(const std::vector< InnerType >& value) {
        return Any::fromObject(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedStdVector*, false >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const std::vector< InnerType >&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedStdVector >(value.getObjectUnsafe());
	}
    static Any set(const std::vector< InnerType >& value) {
        return Any::fromObject(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
        return checked_type_cast< BoxedStdVector*, false >(value.getObject())->unbox< InnerType >();
    }
};

void T_DLLCLASS registerBoxClasses(class IScriptManager* scriptManager);
	
	}
}

#endif	// traktor_script_Boxes_H
