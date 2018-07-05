/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Boxes_H
#define traktor_Boxes_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Class/Any.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Range.h"
#include "Core/Math/Ray3.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS Boxed : public RefCountImpl< ITypedObject >
{
	T_RTTI_CLASS;

public:
	virtual std::wstring toString() const = 0;
};

class T_DLLCLASS BoxedTypeInfo : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedTypeInfo();

	explicit BoxedTypeInfo(const TypeInfo& value);

	std::wstring getName() const;

	int32_t getSize() const;

	int32_t getVersion() const;

	Ref< ITypedObject > createInstance() const;

	static Ref< BoxedTypeInfo > find(const std::wstring& name);

	static Ref< class BoxedRefArray > findAllOf(const BoxedTypeInfo* typeInfo, bool inclusive);

	const TypeInfo& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	const TypeInfo& m_value;
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

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

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

	void set_x(float x) { m_value.x = x; }

	float get_x() const { return m_value.x; }

	void set_y(float y) { m_value.y = y; }

	float get_y() const { return m_value.y; }

	float get_length() const { return m_value.length(); }

	Vector2 add(const BoxedVector2* v) const { return m_value + v->m_value; }

	Vector2 sub(const BoxedVector2* v) const { return m_value - v->m_value; }

	Vector2 mul(const BoxedVector2* v) const { return m_value * v->m_value; }

	Vector2 div(const BoxedVector2* v) const { return m_value / v->m_value; }

	Vector2 add(float v) const { return m_value + v; }

	Vector2 sub(float v) const { return m_value - v; }

	Vector2 mul(float v) const { return m_value * v; }

	Vector2 div(float v) const { return m_value / v; }

	float dot(const BoxedVector2* v) const { return traktor::dot(m_value, v->m_value); }

	Vector2 normalized() const { return m_value.normalized(); }

	Vector2 neg() const { return -m_value; }

	Vector2 perpendicular() const { return m_value.perpendicular(); }

	static Vector2 lerp(const BoxedVector2* a, const BoxedVector2* b, float c) { return traktor::lerp(a->m_value, b->m_value, c); }

	static float distance(const BoxedVector2* a, const BoxedVector2* b) { return (b->m_value - a->m_value).length(); }

	const Vector2& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

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

	void set_x(float v) { m_value.set(0, Scalar(v)); }

	float get_x() const { return m_value.x(); }
	
	void set_y(float v) { m_value.set(1, Scalar(v)); }

	float get_y() const { return m_value.y(); }
	
	void set_z(float v) { m_value.set(2, Scalar(v)); }

	float get_z() const { return m_value.z(); }
	
	void set_w(float v) { m_value.set(3, Scalar(v)); }

	float get_w() const { return m_value.w(); }

	Vector4 get_xyz0() const { return m_value.xyz0(); }

	Vector4 get_xyz1() const { return m_value.xyz1(); }

	float get_length() const { return m_value.length(); }

	void set(float x, float y, float z, float w) { m_value.set(x, y, z, w); }
	
	Vector4 add(const BoxedVector4* v) const { return m_value + v->m_value; }

	Vector4 sub(const BoxedVector4* v) const { return m_value - v->m_value; }

	Vector4 mul(const BoxedVector4* v) const { return m_value * v->m_value; }

	Vector4 div(const BoxedVector4* v) const { return m_value / v->m_value; }

	Vector4 add(float v) const { return m_value + Scalar(v); }

	Vector4 sub(float v) const { return m_value - Scalar(v); }

	Vector4 mul(float v) const { return m_value * Scalar(v); }

	Vector4 div(float v) const { return m_value / Scalar(v); }

	float dot(const BoxedVector4* v) const { return traktor::dot3(m_value, v->m_value); }

	Vector4 cross(const BoxedVector4* v) const { return traktor::cross(m_value, v->m_value); }

	Vector4 normalized() const { return m_value.normalized(); }

	Vector4 neg() const { return -m_value; }

	static Vector4 lerp(const BoxedVector4* a, const BoxedVector4* b, float c) { return traktor::lerp(a->m_value, b->m_value, Scalar(c)); }
	
	static float distance3(const BoxedVector4* a, const BoxedVector4* b) { return (b->m_value - a->m_value).xyz0().length(); }

	static float distance4(const BoxedVector4* a, const BoxedVector4* b) { return (b->m_value - a->m_value).length(); }

	const Vector4& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;
	
	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Vector4 m_value;
};

class T_DLLCLASS BoxedVector4Array : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedVector4Array()
	{
	}

	explicit BoxedVector4Array(uint32_t size)
	:	m_arr(size)
	{
	}

	explicit BoxedVector4Array(const AlignedVector< Vector4 >& arr)
	:	m_arr(arr)
	{
	}

	void reserve(uint32_t capacity);

	void resize(uint32_t size);

	void clear();

	int32_t size() const;

	bool empty() const;

	void push_back(const BoxedVector4* value);

	void pop_back();

	Vector4 front();

	Vector4 back();

	void set(int32_t index, const BoxedVector4* value);

	Vector4 get(int32_t index);

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	const AlignedVector< Vector4 >& unbox() const
	{
		return m_arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	AlignedVector< Vector4 > m_arr;
};

class T_DLLCLASS BoxedQuaternion : public Boxed
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const BoxedVector4* axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const BoxedVector4* from, const BoxedVector4* to);

	explicit BoxedQuaternion(const class BoxedMatrix44* m);
	
	void set_x(float v) { m_value.e.set(0, Scalar(v)); }

	float get_x() const { return m_value.e.x(); }
	
	void set_y(float v) { m_value.e.set(1, Scalar(v)); }

	float get_y() const { return m_value.e.y(); }
	
	void set_z(float v) { m_value.e.set(2, Scalar(v)); }

	float get_z() const { return m_value.e.z(); }
	
	void set_w(float v) { m_value.e.set(3, Scalar(v)); }

	float get_w() const { return m_value.e.w(); }

	Quaternion normalized() const;

	Quaternion inverse() const;

	Quaternion concat(const BoxedQuaternion* q) const;

	Vector4 transform(const BoxedVector4* v) const;

	Vector4 getEulerAngles() const;

	Vector4 getAxisAngle() const;

	static Quaternion fromEulerAngles(float head, float pitch, float bank);

	static Quaternion fromAxisAngle(const BoxedVector4* axisAngle);

	static Quaternion lerp(const BoxedQuaternion* a, const BoxedQuaternion* b, float c) { return traktor::lerp(a->m_value, b->m_value, c); }

	static Quaternion slerp(const BoxedQuaternion* a, const BoxedQuaternion* b, float c) { return traktor::slerp(a->m_value, b->m_value, c); }

	const Quaternion& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

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

	Ref< BoxedVector4 > rayIntersection(
		const Vector4& origin,
		const Vector4& direction
	) const;

	Ref< BoxedVector4 > segmentIntersection(
		const Vector4& a,
		const Vector4& b
	) const;

	static Ref< BoxedVector4 > uniqueIntersectionPoint(
		const Plane& a,
		const Plane& b,
		const Plane& c
	);

	const Plane& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Plane m_value;
};

class T_DLLCLASS BoxedTransform : public Boxed
{
	T_RTTI_CLASS;
	
public:
	BoxedTransform();
	
	explicit BoxedTransform(const Transform& value);
	
	explicit BoxedTransform(const BoxedVector4* translation, const BoxedQuaternion* rotation);

	explicit BoxedTransform(const BoxedMatrix44* m);
	
	const Vector4& get_translation() const { return m_value.translation(); }
	
	const Quaternion& get_rotation() const { return m_value.rotation(); }

	Vector4 get_axisX() const { return m_value.axisX(); }

	Vector4 get_axisY() const { return m_value.axisY(); }

	Vector4 get_axisZ() const { return m_value.axisZ(); }

	Plane get_planeX() const;

	Plane get_planeY() const;

	Plane get_planeZ() const;

	Transform inverse() const;

	Matrix44 toMatrix44() const;

	Transform concat(const BoxedTransform* t) const;

	Vector4 transform(const BoxedVector4* v) const;
	
	static Transform lerp(const BoxedTransform* a, const BoxedTransform* b, float c) { return traktor::lerp(a->m_value, b->m_value, Scalar(c)); }

	const Transform& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Transform m_value;
};

class T_DLLCLASS BoxedAabb2 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAabb2();

	explicit BoxedAabb2(const Aabb2& value);

	explicit BoxedAabb2(const BoxedVector2* min, const BoxedVector2* max);

	const Vector2& min() const { return m_value.mn; }

	const Vector2& max() const { return m_value.mx; }

	bool inside(const BoxedVector2* pt) const { return m_value.inside(pt->unbox()); }

	void contain(const BoxedVector2* pt) { m_value.contain(pt->unbox()); }

	Vector2 getCenter() const { return m_value.getCenter(); }

	Vector2 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	const Aabb2& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Aabb2 m_value;
};

class T_DLLCLASS BoxedAabb3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAabb3();

	explicit BoxedAabb3(const Aabb3& value);

	explicit BoxedAabb3(const BoxedVector4* min, const BoxedVector4* max);

	const Vector4& min() const { return m_value.mn; }

	const Vector4& max() const { return m_value.mx; }

	bool inside(const BoxedVector4* pt) const { return m_value.inside(pt->unbox()); }

	Aabb3 transform(const BoxedTransform* tf) const { return m_value.transform(tf->unbox()); }

	void contain(const BoxedVector4* pt) { m_value.contain(pt->unbox()); }

	Aabb3 scale(const Scalar& factor) const { return m_value.scale(factor); }

	Aabb3 expand(const Scalar& margin) const { return m_value.expand(margin); }

	const Vector4& getMin() const { return m_value.mn; }

	const Vector4& getMax() const { return m_value.mx; }

	Vector4 getCenter() const { return m_value.getCenter(); }

	Vector4 getExtent() const { return m_value.getExtent(); }

	bool empty() const { return m_value.empty(); }

	bool overlap(const BoxedAabb3* aabb) const { return m_value.overlap(aabb->unbox()); }

	Any intersectRay(const BoxedVector4* origin, const BoxedVector4* direction) const;

	const Aabb3& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Aabb3 m_value;
};

class T_DLLCLASS BoxedFrustum : public Boxed
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

	bool insidePoint(const BoxedVector4* point) const;

	int32_t insideSphere(const BoxedVector4* center, float radius) const;

	int32_t insideAabb(const BoxedAabb3* aabb) const;

	const Plane& getPlane(int32_t index) const;

	const Vector4& getCorner(int32_t index) const;

	const Vector4& getCenter() const;

	const Frustum& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Frustum m_value;
};

class T_DLLCLASS BoxedMatrix33 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedMatrix33();

	explicit BoxedMatrix33(const Matrix33& value);

	Vector4 diagonal() const;

	float determinant() const;

	Matrix33 transpose() const;

	Matrix33 inverse() const;

	void set(int r, int c, float v);

	float get(int r, int c) const;

	Matrix33 concat(const BoxedMatrix33* t) const;

	Vector2 transform(const BoxedVector2* v) const;

	static Matrix33 translate(float x, float y);

	static Matrix33 scale(float x, float y);

	static Matrix33 rotate(float angle);

	const Matrix33& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Matrix33 m_value;
};

class T_DLLCLASS BoxedMatrix44 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedMatrix44();

	explicit BoxedMatrix44(const Matrix44& value);

	explicit BoxedMatrix44(const BoxedVector4* axisX, const BoxedVector4* axisY, const BoxedVector4* axisZ, const BoxedVector4* translation);

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

	void setColumn(int c, const BoxedVector4* v);

	Vector4 getColumn(int c);

	void setRow(int r, const BoxedVector4* v);

	Vector4 getRow(int r);

	void set(int r, int c, float v);

	float get(int r, int c) const;

	Matrix44 concat(const BoxedMatrix44* t) const;

	Vector4 transform(const BoxedVector4* v) const;

	const Matrix44& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Matrix44 m_value;
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

	static Color4f lerp(const BoxedColor4f* a, const BoxedColor4f* b, float c) { return Color4f(traktor::lerp(a->m_value, b->m_value, Scalar(c))); }

	const Color4f& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

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

	std::wstring formatRGB() const;

	std::wstring formatARGB() const;

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Color4ub m_value;
};

class T_DLLCLASS BoxedRandom : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRandom();

	explicit BoxedRandom(const Random& value);

	explicit BoxedRandom(uint32_t seed);

	uint32_t next() { return m_value.next(); }

	float nextFloat() { return m_value.nextFloat(); }

	const Random& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE;

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

	Vector4 nextHemi(const BoxedVector4* direction) { return m_value.nextHemi(direction->unbox()); }

	const RandomGeometry& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RandomGeometry m_value;
};

class T_DLLCLASS BoxedRay3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRay3();

	explicit BoxedRay3(const Ray3& value);

	const Vector4& origin() const { return m_value.origin; }

	const Vector4& direction() const { return m_value.direction; }

	float distance(const BoxedVector4* v) { return m_value.distance(v->unbox()); }

	Vector4 mul(float f) const { return m_value * Scalar(f); }

	const Ray3& unbox() const { return m_value; }

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Ray3 m_value;
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

	void set(int32_t index, ITypedObject* object);

	ITypedObject* get(int32_t index);

	void push_back(ITypedObject* object);

	void pop_back();

	ITypedObject* front();

	ITypedObject* back();

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	template < typename ObjectType >
	RefArray< ObjectType > unbox() const
	{
		RefArray< ObjectType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = mandatory_non_null_type_cast< ObjectType* >(m_arr[i]);
		return arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RefArray< ITypedObject > m_arr;
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

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

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

class T_DLLCLASS BoxedAlignedVector : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAlignedVector();

	explicit BoxedAlignedVector(uint32_t size);

	template < typename ItemType >
	BoxedAlignedVector(const AlignedVector< ItemType >& arr)
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

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

	template < typename ItemType >
	AlignedVector< ItemType > unbox() const
	{
		AlignedVector< ItemType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = CastAny< ItemType >::get(m_arr[i]);
		return arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	AlignedVector< Any > m_arr;
};

class T_DLLCLASS BoxedStdVector : public Boxed
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

	virtual std::wstring toString() const T_OVERRIDE T_FINAL;

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
struct CastAny< TypeInfo, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.TypeInfo";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTypeInfo >(value.getObjectUnsafe());
	}
	static Any set(const TypeInfo& value) {
		return Any::fromObject(new BoxedTypeInfo(value));
	}
	static const TypeInfo& get(const Any& value) {
		return static_cast< BoxedTypeInfo* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny< const TypeInfo&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const TypeInfo&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTypeInfo >(value.getObjectUnsafe());
	}
	static Any set(const TypeInfo& value) {
		return Any::fromObject(new BoxedTypeInfo(value));
	}
	static const TypeInfo& get(const Any& value) {
		return static_cast< BoxedTypeInfo* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Guid, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Guid";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}	
	static const Guid& get(const Any& value) {
		return static_cast< BoxedGuid* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Guid&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Guid&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}	
	static const Guid& get(const Any& value) {
		return static_cast< BoxedGuid* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Scalar, false >
{
	T_NO_COPY_CLASS(CastAny);
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"Scalar";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < const Scalar&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const Scalar&";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < Vector2, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Vector2";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector2 >(value.getObjectUnsafe());
	}
	static Any set(const Vector2& value) {
		return Any::fromObject(new BoxedVector2(value));
	}	
	static const Vector2& get(const Any& value) {
		return static_cast< BoxedVector2* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Vector2&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Vector2&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector2 >(value.getObjectUnsafe());
	}
	static Any set(const Vector2& value) {
		return Any::fromObject(new BoxedVector2(value));
	}	
	static const Vector2& get(const Any& value) {
		return static_cast< BoxedVector2* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Vector4, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Vector4";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4 >(value.getObjectUnsafe());
	}
    static Any set(const Vector4& value) {
        return Any::fromObject(new BoxedVector4(value));
    }	
    static const Vector4& get(const Any& value) {
		return static_cast< BoxedVector4* >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Vector4&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Vector4&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4 >(value.getObjectUnsafe());
	}
    static Any set(const Vector4& value) {
        return Any::fromObject(new BoxedVector4(value));
    }	
    static const Vector4& get(const Any& value) {
		return static_cast< BoxedVector4* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < AlignedVector< Vector4 >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.AlignedVector< traktor.Vector4 >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4Array >(value.getObjectUnsafe());
	}
	static Any set(const AlignedVector< Vector4 >& value) {
		return Any::fromObject(new BoxedVector4Array(value));
	}
	static const AlignedVector< Vector4 >& get(const Any& value) {
		return static_cast< BoxedVector4Array* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const AlignedVector< Vector4 >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.AlignedVector< traktor.Vector4 >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4Array >(value.getObjectUnsafe());
	}
	static Any set(const AlignedVector< Vector4 >& value) {
		return Any::fromObject(new BoxedVector4Array(value));
	}
	static const AlignedVector< Vector4 >& get(const Any& value) {
		return static_cast< BoxedVector4Array* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Quaternion, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Quaternion";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static const Quaternion& get(const Any& value) {
		return static_cast< BoxedQuaternion* >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Quaternion&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Quaternion&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static const Quaternion& get(const Any& value) {
		return static_cast< BoxedQuaternion* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Plane, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Plane";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static const Plane& get(const Any& value) {
		return static_cast< BoxedPlane* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Plane&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Plane&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPlane >(value.getObjectUnsafe());
	}
	static Any set(const Plane& value) {
		return Any::fromObject(new BoxedPlane(value));
	}
	static const Plane& get(const Any& value) {
		return static_cast< BoxedPlane* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Transform, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Transform";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTransform >(value.getObjectUnsafe());
	}
    static Any set(const Transform& value) {
        return Any::fromObject(new BoxedTransform(value));
    }
    static const Transform& get(const Any& value) {
		return static_cast< BoxedTransform* >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < const Transform&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Transform&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTransform >(value.getObjectUnsafe());
	}
    static Any set(const Transform& value) {
        return Any::fromObject(new BoxedTransform(value));
    }
    static const Transform& get(const Any& value) {
		return static_cast< BoxedTransform* >(value.getObject())->unbox();
    }
};

template < >
struct CastAny < Aabb2, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Aabb2";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb2 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb2& value) {
		return Any::fromObject(new BoxedAabb2(value));
	}	
	static const Aabb2& get(const Any& value) {
		return static_cast< BoxedAabb2* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Aabb2&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Aabb2&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb2 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb2& value) {
		return Any::fromObject(new BoxedAabb2(value));
	}	
	static const Aabb2& get(const Any& value) {
		return static_cast< BoxedAabb2* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Aabb3, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Aabb3";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}	
	static const Aabb3& get(const Any& value) {
		return static_cast< BoxedAabb3* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Aabb3&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Aabb3&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAabb3 >(value.getObjectUnsafe());
	}
	static Any set(const Aabb3& value) {
		return Any::fromObject(new BoxedAabb3(value));
	}	
	static const Aabb3& get(const Any& value) {
		return static_cast< BoxedAabb3* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Frustum, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Frustum";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}	
	static const Frustum& get(const Any& value) {
		return static_cast< BoxedFrustum* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Frustum&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Frustum&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedFrustum >(value.getObjectUnsafe());
	}
	static Any set(const Frustum& value) {
		return Any::fromObject(new BoxedFrustum(value));
	}	
	static const Frustum& get(const Any& value) {
		return static_cast< BoxedFrustum* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Matrix33, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Matrix33";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix33 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix33& value) {
		return Any::fromObject(new BoxedMatrix33(value));
	}	
	static const Matrix33& get(const Any& value) {
		return static_cast< BoxedMatrix33* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Matrix33&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Matrix33&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix33 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix33& value) {
		return Any::fromObject(new BoxedMatrix33(value));
	}	
	static const Matrix33& get(const Any& value) {
		return static_cast< BoxedMatrix33* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Matrix44, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Matrix44";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}	
	static const Matrix44& get(const Any& value) {
		return static_cast< BoxedMatrix44* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Matrix44&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Matrix44&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedMatrix44 >(value.getObjectUnsafe());
	}
	static Any set(const Matrix44& value) {
		return Any::fromObject(new BoxedMatrix44(value));
	}	
	static const Matrix44& get(const Any& value) {
		return static_cast< BoxedMatrix44* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Color4f, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Color4f";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4f >(value.getObjectUnsafe());
	}
	static Any set(const Color4f& value) {
		return Any::fromObject(new BoxedColor4f(value));
	}
	static const Color4f& get(const Any& value) {
		return static_cast< BoxedColor4f* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Color4f&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Color4f&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4f >(value.getObjectUnsafe());
	}
	static Any set(const Color4f& value) {
		return Any::fromObject(new BoxedColor4f(value));
	}
	static const Color4f& get(const Any& value) {
		return static_cast< BoxedColor4f* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Color4ub, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Color4ub";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static const Color4ub& get(const Any& value) {
		return static_cast< BoxedColor4ub* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Color4ub&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Color4ub&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static const Color4ub& get(const Any& value) {
		return static_cast< BoxedColor4ub* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Random, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Random";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static const Random& get(const Any& value) {
		return static_cast< BoxedRandom* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Random&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Random&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandom >(value.getObjectUnsafe());
	}
	static Any set(const Random& value) {
		return Any::fromObject(new BoxedRandom(value));
	}
	static const Random& get(const Any& value) {
		return static_cast< BoxedRandom* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < RandomGeometry, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.RandomGeometry";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static const RandomGeometry& get(const Any& value) {
		return static_cast< BoxedRandomGeometry* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const RandomGeometry&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.RandomGeometry&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static const RandomGeometry& get(const Any& value) {
		return static_cast< BoxedRandomGeometry* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Ray3, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Ray3";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRay3 >(value.getObjectUnsafe());
	}
	static Any set(const Ray3& value) {
		return Any::fromObject(new BoxedRay3(value));
	}
	static const Ray3& get(const Any& value) {
		return static_cast< BoxedRay3* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Ray3&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Ray3&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRay3 >(value.getObjectUnsafe());
	}
	static Any set(const Ray3& value) {
		return Any::fromObject(new BoxedRay3(value));
	}
	static const Ray3& get(const Any& value) {
		return static_cast< BoxedRay3* >(value.getObject())->unbox();
	}
};

template < typename InnerType >
struct CastAny< Range< InnerType >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Range< InnerType >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return static_cast< BoxedRange* >(value.getObject())->unbox< InnerType >();
	}
};

template < typename InnerType >
struct CastAny< const Range< InnerType >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Range< InnerType >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return static_cast< BoxedRange* >(value.getObject())->unbox< InnerType >();
	}
};

template < typename InnerType >
struct CastAny < RefArray< InnerType >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.RefArray< " << type_name< InnerType >() << L" >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
		return static_cast< BoxedRefArray* >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const RefArray< InnerType >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.RefArray< " << type_name< InnerType >() << L" >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
		return static_cast< BoxedRefArray* >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < AlignedVector< InnerType >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.AlignedVector< InnerType >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAlignedVector >(value.getObjectUnsafe());
	}
    static Any set(const AlignedVector< InnerType >& value) {
        return Any::fromObject(new BoxedAlignedVector(value));
    }
    static AlignedVector< InnerType > get(const Any& value) {
		return static_cast< BoxedAlignedVector* >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const AlignedVector< InnerType >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.AlignedVector< InnerType >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAlignedVector >(value.getObjectUnsafe());
	}
    static Any set(const AlignedVector< InnerType >& value) {
        return Any::fromObject(new BoxedAlignedVector(value));
    }
    static AlignedVector< InnerType > get(const Any& value) {
		return static_cast< BoxedAlignedVector* >(value.getObject())->unbox< InnerType >();
	}
};

template < typename InnerType >
struct CastAny < std::vector< InnerType >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.StdVector< InnerType >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedStdVector >(value.getObjectUnsafe());
	}
    static Any set(const std::vector< InnerType >& value) {
        return Any::fromObject(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
		return static_cast< BoxedStdVector* >(value.getObject())->unbox< InnerType >();
    }
};

template < typename InnerType >
struct CastAny < const std::vector< InnerType >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.StdVector< InnerType >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedStdVector >(value.getObjectUnsafe());
	}
    static Any set(const std::vector< InnerType >& value) {
        return Any::fromObject(new BoxedStdVector(value));
    }
    static std::vector< InnerType > get(const Any& value) {
		return static_cast< BoxedStdVector* >(value.getObject())->unbox< InnerType >();
	}
};

class T_DLLCLASS BoxesClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const T_OVERRIDE T_FINAL;
};
	
}

#endif	// traktor_Boxes_H
