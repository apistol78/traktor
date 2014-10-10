#ifndef traktor_script_Boxes_H
#define traktor_script_Boxes_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/RandomGeometry.h"
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

class T_DLLCLASS Boxed : public ITypedObject
{
	T_RTTI_CLASS;

public:
	virtual std::wstring toString() const = 0;
};

class T_DLLCLASS BoxedUInt64 : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedUInt64();

	explicit BoxedUInt64(uint64_t value);

	std::wstring format() const;

	uint64_t unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	uint64_t m_value;
};

class T_DLLCLASS BoxedGuid : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Guid m_value;
};

class T_DLLCLASS BoxedVector2 : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedVector2();

	explicit BoxedVector2(const Vector2& value);

	explicit BoxedVector2(float x, float y);

	void set(float x, float y) { m_value.set(x, y); }

	float x() const { return m_value.x; }

	float y() const { return m_value.y; }

	Vector2 add(const Vector2& v) const { return m_value + v; }

	Vector2 sub(const Vector2& v) const { return m_value - v; }

	Vector2 mul(const Vector2& v) const { return m_value * v; }

	Vector2 div(const Vector2& v) const { return m_value / v; }

	Vector2 add(float v) const { return m_value + v; }

	Vector2 sub(float v) const { return m_value - v; }

	Vector2 mul(float v) const { return m_value * v; }

	Vector2 div(float v) const { return m_value / v; }

	float dot(const Vector2& v) const { return traktor::dot(m_value, v); }

	float length() const { return m_value.length(); }

	Vector2 normalized() const { return m_value.normalized(); }

	Vector2 neg() const { return -m_value; }

	Vector2 perpendicular() const { return m_value.perpendicular(); }

	static Vector2 zero() { return Vector2::zero(); }

	static Vector2 lerp(const Vector2& a, const Vector2& b, float c) { return traktor::lerp(a, b, c); }

	const Vector2& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Vector2 m_value;
};

class T_DLLCLASS BoxedVector4 : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;
	
public:
	BoxedVector4();
	
	explicit BoxedVector4(const Vector4& value);

	explicit BoxedVector4(float x, float y, float z);
	
	explicit BoxedVector4(float x, float y, float z, float w);

	void set(float x, float y, float z, float w) { m_value.set(x, y, z, w); }
	
	float x() const { return m_value.x(); }
	
	float y() const { return m_value.y(); }
	
	float z() const { return m_value.z(); }
	
	float w() const { return m_value.w(); }

	Vector4 xyz0() const { return m_value.xyz0(); }

	Vector4 xyz1() const { return m_value.xyz1(); }

	Vector4 add(const Vector4& v) const { return m_value + v; }

	Vector4 sub(const Vector4& v) const { return m_value - v; }

	Vector4 mul(const Vector4& v) const { return m_value * v; }

	Vector4 div(const Vector4& v) const { return m_value / v; }

	Vector4 add(float v) const { return m_value + Scalar(v); }

	Vector4 sub(float v) const { return m_value - Scalar(v); }

	Vector4 mul(float v) const { return m_value * Scalar(v); }

	Vector4 div(float v) const { return m_value / Scalar(v); }

	float dot(const Vector4& v) const { return traktor::dot3(m_value, v); }

	Vector4 cross(const Vector4& v) const { return traktor::cross(m_value, v); }

	float length() const { return m_value.length(); }

	Vector4 normalized() const { return m_value.normalized(); }

	Vector4 neg() const { return -m_value; }

	static Vector4 zero() { return Vector4::zero(); }

	static Vector4 origo() { return Vector4::origo(); }

	static Vector4 lerp(const Vector4& a, const Vector4& b, float c) { return traktor::lerp(a, b, Scalar(c)); }
	
	const Vector4& unbox() const { return m_value; }

	virtual std::wstring toString() const;
	
	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Vector4 m_value;
};

class T_DLLCLASS BoxedQuaternion : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const Vector4& axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const Vector4& from, const Vector4& to);

	explicit BoxedQuaternion(const Matrix44& m);
	
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

	static Quaternion slerp(const Quaternion& a, const Quaternion& b, float c) { return traktor::slerp(a, b, c); }

	const Quaternion& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Quaternion m_value;
};

class T_DLLCLASS BoxedPlane : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Plane m_value;
};

class T_DLLCLASS BoxedTransform : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;
	
public:
	BoxedTransform();
	
	explicit BoxedTransform(const Transform& value);
	
	explicit BoxedTransform(const Vector4& translation, const Quaternion& rotation);

	explicit BoxedTransform(const Matrix44& m);
	
	const Vector4& translation() const;
	
	const Quaternion& rotation() const;

	Vector4 axisX() const;

	Vector4 axisY() const;

	Vector4 axisZ() const;

	Plane planeX() const;

	Plane planeY() const;

	Plane planeZ() const;

	Transform inverse() const;

	Matrix44 toMatrix44() const;

	Transform concat(const Transform& t) const;

	Vector4 transform(const Vector4& v) const;
	
	static Transform identity() { return Transform::identity(); }

	static Transform lerp(const Transform& a, const Transform& b, float c) { return traktor::lerp(a, b, Scalar(c)); }

	const Transform& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Transform m_value;
};

class T_DLLCLASS BoxedAabb3 : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedAabb3();

	explicit BoxedAabb3(const Aabb3& value);

	explicit BoxedAabb3(const Vector4& min, const Vector4& max);

	bool inside(const Vector4& pt) const { return m_value.inside(pt); }

	Aabb3 transform(const Transform& tf) const { return m_value.transform(tf); }

	void contain(const Vector4& pt) { m_value.contain(pt); }

	Aabb3 scale(const Scalar& factor) const { return m_value.scale(factor); }

	Aabb3 expand(const Scalar& margin) const { return m_value.expand(margin); }

	const Vector4& getMin() const { return m_value.mn; }

	const Vector4& getMax() const { return m_value.mx; }

	Vector4 getCenter() const { return m_value.getCenter(); }

	Vector4 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	bool overlap(const Aabb3& aabb) const { return m_value.overlap(aabb); }

	Any intersectRay(const Vector4& origin, const Vector4& direction) const;

	const Aabb3& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Aabb3 m_value;
};

class T_DLLCLASS BoxedFrustum : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedFrustum();

	explicit BoxedFrustum(const Frustum& value);

	void buildPerspective(float vfov, float aspect, float zn, float zf);

	void buildOrtho(float width, float height, float zn, float zf);

	void setNearZ(float zn);

	float getNearZ() const;

	void setFarZ(float zf);

	float getFarZ() const;

	bool insidePoint(const Vector4& point) const;

	int32_t insideSphere(const Vector4& center, float radius) const;

	int32_t insideAabb(const Aabb3& aabb) const;

	const Plane& getPlane(int32_t index) const;

	const Vector4& getCorner(int32_t index) const;

	const Vector4& getCenter() const;

	const Frustum& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Frustum m_value;
};

class T_DLLCLASS BoxedMatrix44 : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedMatrix44();

	explicit BoxedMatrix44(const Matrix44& value);

	explicit BoxedMatrix44(const Vector4& axisX, const Vector4& axisY, const Vector4& axisZ, const Vector4& translation);

	Vector4 axisX() const;

	Vector4 axisY() const;

	Vector4 axisZ() const;

	Plane planeX() const;

	Plane planeY() const;

	Plane planeZ() const;

	Vector4 translation() const;

	Vector4 diagonal() const;

	bool isOrtho() const;

	float determinant() const;

	Matrix44 transpose() const;

	Matrix44 inverse() const;

	void setColumn(int c, const Vector4& v);

	Vector4 getColumn(int c);

	void setRow(int r, const Vector4& v);

	Vector4 getRow(int r);

	void set(int r, int c, float v);

	float get(int r, int c) const;

	Matrix44 concat(const Matrix44& t) const;

	Vector4 transform(const Vector4& v) const;

	static Matrix44 zero() { return Matrix44::zero(); }

	static Matrix44 identity() { return Matrix44::identity(); }

	const Matrix44& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Matrix44 m_value;
};

class T_DLLCLASS BoxedColor4f : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Color4f m_value;
};

class T_DLLCLASS BoxedColor4ub : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Color4ub m_value;
};

class T_DLLCLASS BoxedRandom : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedRandom();

	explicit BoxedRandom(const Random& value);

	explicit BoxedRandom(uint32_t seed);

	uint32_t next() { return m_value.next(); }

	float nextFloat() { return m_value.nextFloat(); }

	const Random& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Random m_value;
};

class T_DLLCLASS BoxedRandomGeometry : public BoxedRandom
{
	T_RTTI_CLASS;

public:
	BoxedRandomGeometry();

	explicit BoxedRandomGeometry(const RandomGeometry& value);

	explicit BoxedRandomGeometry(uint32_t seed);

	Vector4 nextUnit() { return m_value.nextUnit(); }

	Vector4 nextHemi(const Vector4& direction) { return m_value.nextHemi(direction); }

	const RandomGeometry& unbox() const { return m_value; }

	virtual std::wstring toString() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RandomGeometry m_value;
};

class T_DLLCLASS BoxedRefArray : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RefArray< Object > m_arr;
};

class T_DLLCLASS BoxedRange : public RefCountImpl< Boxed >
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

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Any m_min;
	Any m_max;
};

class T_DLLCLASS BoxedStdVector : public RefCountImpl< Boxed >
{
	T_RTTI_CLASS;

public:
	BoxedStdVector();

	explicit BoxedStdVector(uint32_t size);

	template < typename ItemType >
	BoxedStdVector(const std::vector< ItemType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = CastAny< ItemType >::set(arr[i]);
	}

	void reserve(uint32_t capacity);

	void resize(uint32_t size);

	void clear();

	int32_t size() const;

	bool empty() const;

	void push_back(const Any& value);

	void pop_back();

	const Any& front();

	const Any& back();

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

	void* operator new (size_t size);

	void operator delete (void* ptr);

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
struct CastAny < Frustum, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}	
	static Frustum get(const Any& value) {
		return checked_type_cast< BoxedFrustum*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Frustum&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}	
	static Frustum get(const Any& value) {
		return checked_type_cast< BoxedFrustum*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Matrix44, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}	
	static Matrix44 get(const Any& value) {
		return checked_type_cast< BoxedMatrix44*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Matrix44&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}	
	static Matrix44 get(const Any& value) {
		return checked_type_cast< BoxedMatrix44*, false >(value.getObject())->unbox();
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

template < >
struct CastAny < Random, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static Random get(const Any& value) {
		return checked_type_cast< BoxedRandom*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Random&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static Random get(const Any& value) {
		return checked_type_cast< BoxedRandom*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < RandomGeometry, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static RandomGeometry get(const Any& value) {
		return checked_type_cast< BoxedRandomGeometry*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const RandomGeometry&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static RandomGeometry get(const Any& value) {
		return checked_type_cast< BoxedRandomGeometry*, false >(value.getObject())->unbox();
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
