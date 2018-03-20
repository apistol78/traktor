/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/SpinLock.h"

namespace traktor
{
	namespace
	{

template < typename BoxedType, int BoxesPerBlock >
class BoxedAllocator
{
public:
	void* alloc()
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif

		void* ptr = 0;
		for (std::vector< BlockAllocator* >::iterator i = m_allocators.begin(); i != m_allocators.end(); ++i)
		{
			BlockAllocator* allocator = *i;
			if ((ptr = allocator->alloc()) != 0)
				return ptr;
		}

		// No more space in block allocators; create a new block allocator.
		void* top = Alloc::acquireAlign(BoxesPerBlock * sizeof(BoxedType), alignOf< BoxedType >(), T_FILE_LINE);
		T_FATAL_ASSERT_M (top, L"Out of memory");

		BlockAllocator* allocator = new BlockAllocator(top, BoxesPerBlock, sizeof(BoxedType));
		T_FATAL_ASSERT_M (allocator, L"Out of memory");

		m_allocators.push_back(allocator);

		T_DEBUG(L"Alloc " << BoxesPerBlock << L" of " << type_of< BoxedType >().getName() << L"; " << uint32_t(BoxesPerBlock * sizeof(BoxedType)) << L" bytes");
		T_DEBUG(L"  " << int32_t(m_allocators.size()) << L" chunk(s)");

		return allocator->alloc();
	}

	void free(void* ptr)
	{
#if defined(T_BOXES_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
#endif

		for (std::vector< BlockAllocator* >::iterator i = m_allocators.begin(); i != m_allocators.end(); ++i)
		{
			BlockAllocator* allocator = *i;
			if (allocator->free(ptr))
				return;
		}

		T_FATAL_ERROR;
	}

private:
#if defined(T_BOXES_USE_MT_LOCK)
	SpinLock m_lock;
#endif
	std::vector< BlockAllocator* > m_allocators;
};

BoxedAllocator< BoxedTypeInfo, 16 > s_allocBoxedTypeInfo;
BoxedAllocator< BoxedGuid, 512 > s_allocBoxedGuid;
BoxedAllocator< BoxedVector2, 1024 > s_allocBoxedVector2;
#if !defined(__ANDROID__) && !defined(__IOS__) && !defined(__PS3__) && !defined(__EMSCRIPTEN__)
BoxedAllocator< BoxedVector4, 32768 > s_allocBoxedVector4;
BoxedAllocator< BoxedVector4Array, 16 > s_allocBoxedVector4Array;
BoxedAllocator< BoxedQuaternion, 4096 > s_allocBoxedQuaternion;
#else
BoxedAllocator< BoxedVector4, 1024 > s_allocBoxedVector4;
BoxedAllocator< BoxedVector4Array, 16 > s_allocBoxedVector4Array;
BoxedAllocator< BoxedQuaternion, 256 > s_allocBoxedQuaternion;
#endif
BoxedAllocator< BoxedPlane, 256 > s_allocBoxedPlane;
BoxedAllocator< BoxedTransform, 8192 > s_allocBoxedTransform;
BoxedAllocator< BoxedAabb2, 64 > s_allocBoxedAabb2;
BoxedAllocator< BoxedAabb3, 64 > s_allocBoxedAabb3;
BoxedAllocator< BoxedFrustum, 16 > s_allocBoxedFrustum;
BoxedAllocator< BoxedMatrix33, 256 > s_allocBoxedMatrix33;
BoxedAllocator< BoxedMatrix44, 16 > s_allocBoxedMatrix44;
BoxedAllocator< BoxedColor4f, 16 > s_allocBoxedColor4f;
BoxedAllocator< BoxedColor4ub, 16 > s_allocBoxedColor4ub;
BoxedAllocator< BoxedRandom, 4 > s_allocBoxedRandom;
BoxedAllocator< BoxedRandomGeometry, 4 > s_allocBoxedRandomGeometry;
BoxedAllocator< BoxedRefArray, 512 > s_allocBoxedRefArray;
BoxedAllocator< BoxedRange, 256 > s_allocBoxedRange;
BoxedAllocator< BoxedAlignedVector, 16 > s_allocBoxedAlignedVector;
BoxedAllocator< BoxedStdVector, 16 > s_allocBoxedStdVector;
BoxedAllocator< BoxedRay3, 8 > s_allocBoxedRay3;

	}


T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Boxed", Boxed)


T_IMPLEMENT_RTTI_CLASS(L"traktor.TypeInfo", BoxedTypeInfo, Boxed)

BoxedTypeInfo::BoxedTypeInfo()
:	m_value(*TypeInfo::find(L"traktor.Object"))
{
}

BoxedTypeInfo::BoxedTypeInfo(const TypeInfo& value)
:	m_value(value)
{
}

std::wstring BoxedTypeInfo::getName() const
{
	return m_value.getName();
}

int32_t BoxedTypeInfo::getSize() const
{
	return (int32_t)m_value.getSize();
}

int32_t BoxedTypeInfo::getVersion() const
{
	return m_value.getVersion();
}

Ref< ITypedObject > BoxedTypeInfo::createInstance() const
{
	return m_value.createInstance();
}

Ref< BoxedTypeInfo > BoxedTypeInfo::find(const std::wstring& name)
{
	const TypeInfo* type = TypeInfo::find(name);
	return type != 0 ? new BoxedTypeInfo(*type) : 0;
}

Ref< BoxedRefArray > BoxedTypeInfo::findAllOf(const BoxedTypeInfo* typeInfo, bool inclusive)
{
	std::set< const TypeInfo* > types;
	typeInfo->unbox().findAllOf(types, inclusive);

	Ref< BoxedRefArray > boxedTypes = new BoxedRefArray();
	for (std::set< const TypeInfo* >::const_iterator i = types.begin(); i != types.end(); ++i)
		boxedTypes->push_back(new BoxedTypeInfo(*(*i)));
	
	return boxedTypes;
}

std::wstring BoxedTypeInfo::toString() const
{
	return m_value.getName();
}

void* BoxedTypeInfo::operator new (size_t size)
{
	return s_allocBoxedTypeInfo.alloc();
}

void BoxedTypeInfo::operator delete (void* ptr)
{
	s_allocBoxedTypeInfo.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Guid", BoxedGuid, Boxed)

BoxedGuid::BoxedGuid()
{
}

BoxedGuid::BoxedGuid(const Guid& value)
:	m_value(value)
{
}

BoxedGuid::BoxedGuid(const std::wstring& value)
:	m_value(value)
{
}

std::wstring BoxedGuid::toString() const
{
	return m_value.format();
}

void* BoxedGuid::operator new (size_t size)
{
	return s_allocBoxedGuid.alloc();
}

void BoxedGuid::operator delete (void* ptr)
{
	s_allocBoxedGuid.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector2", BoxedVector2, Boxed)

BoxedVector2::BoxedVector2()
:	m_value(0.0f, 0.0f)
{
}

BoxedVector2::BoxedVector2(const Vector2& value)
:	m_value(value)
{
}

BoxedVector2::BoxedVector2(float x, float y)
:	m_value(x, y)
{
}

std::wstring BoxedVector2::toString() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void* BoxedVector2::operator new (size_t size)
{
	return s_allocBoxedVector2.alloc();
}

void BoxedVector2::operator delete (void* ptr)
{
	s_allocBoxedVector2.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4", BoxedVector4, Boxed)

BoxedVector4::BoxedVector4()
:	m_value(Vector4::zero())
{
}

BoxedVector4::BoxedVector4(const Vector4& value)
:	m_value(value)
{
}

BoxedVector4::BoxedVector4(float x, float y, float z)
:	m_value(x, y, z, 0.0f)
{
}

BoxedVector4::BoxedVector4(float x, float y, float z, float w)
:	m_value(x, y, z, w)
{
}

std::wstring BoxedVector4::toString() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void* BoxedVector4::operator new (size_t size)
{
	return s_allocBoxedVector4.alloc();
}

void BoxedVector4::operator delete (void* ptr)
{
	s_allocBoxedVector4.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4Array", BoxedVector4Array, Boxed)

void BoxedVector4Array::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedVector4Array::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedVector4Array::clear()
{
	m_arr.clear();
}

int32_t BoxedVector4Array::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedVector4Array::empty() const
{
	return m_arr.empty();
}

void BoxedVector4Array::push_back(const BoxedVector4* value)
{
	m_arr.push_back(value->unbox());
}

void BoxedVector4Array::pop_back()
{
	m_arr.pop_back();
}

Vector4 BoxedVector4Array::front()
{
	return m_arr.front();
}

Vector4 BoxedVector4Array::back()
{
	return m_arr.back();
}

void BoxedVector4Array::set(int32_t index, const BoxedVector4* value)
{
	m_arr[index] = value->unbox();
}

Vector4 BoxedVector4Array::get(int32_t index)
{
	return m_arr[index];
}

std::wstring BoxedVector4Array::toString() const
{
	return L"(Vector4 array)";
}

void* BoxedVector4Array::operator new (size_t size)
{
	return s_allocBoxedVector4Array.alloc();
}

void BoxedVector4Array::operator delete (void* ptr)
{
	s_allocBoxedVector4Array.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Quaternion", BoxedQuaternion, Boxed)

BoxedQuaternion::BoxedQuaternion()
:	m_value(Quaternion::identity())
{
}

BoxedQuaternion::BoxedQuaternion(const Quaternion& value)
:	m_value(value)
{
}

BoxedQuaternion::BoxedQuaternion(float x, float y, float z, float w)
:	m_value(x, y, z, w)
{
}

BoxedQuaternion::BoxedQuaternion(const BoxedVector4* axis, float angle)
{
	m_value = Quaternion::fromAxisAngle(axis->unbox(), Scalar(angle));
}

BoxedQuaternion::BoxedQuaternion(float head, float pitch, float bank)
{
	m_value = Quaternion::fromEulerAngles(head, pitch, bank);
}

BoxedQuaternion::BoxedQuaternion(const BoxedVector4* from, const BoxedVector4* to)
:	m_value(from->unbox(), to->unbox())
{
}

BoxedQuaternion::BoxedQuaternion(const BoxedMatrix44* m)
:	m_value(m->unbox())
{
}

Quaternion BoxedQuaternion::normalized() const
{
	return m_value.normalized();
}

Quaternion BoxedQuaternion::inverse() const
{
	return m_value.inverse();
}

Quaternion BoxedQuaternion::concat(const BoxedQuaternion* q) const
{
	return m_value * q->m_value;
}

Vector4 BoxedQuaternion::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

Vector4 BoxedQuaternion::getEulerAngles() const
{
	return m_value.toEulerAngles();
}

Vector4 BoxedQuaternion::getAxisAngle() const
{
	return m_value.toAxisAngle();
}

Quaternion BoxedQuaternion::fromEulerAngles(float head, float pitch, float bank)
{
	return Quaternion::fromEulerAngles(head, pitch, bank);
}

Quaternion BoxedQuaternion::fromAxisAngle(const BoxedVector4* axisAngle)
{
	return Quaternion::fromAxisAngle(axisAngle->unbox());
}

std::wstring BoxedQuaternion::toString() const
{
	StringOutputStream ss;
	ss << m_value.e;
	return ss.str();
}

void* BoxedQuaternion::operator new (size_t size)
{
	return s_allocBoxedQuaternion.alloc();
}

void BoxedQuaternion::operator delete (void* ptr)
{
	s_allocBoxedQuaternion.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Plane", BoxedPlane, Boxed)

BoxedPlane::BoxedPlane()
{
}

BoxedPlane::BoxedPlane(const Plane& value)
:	m_value(value)
{
}

BoxedPlane::BoxedPlane(const Vector4& normal, float distance)
:	m_value(normal, Scalar(distance))
{
}

BoxedPlane::BoxedPlane(const Vector4& normal, const Vector4& pointInPlane)
:	m_value(normal, pointInPlane)
{
}

BoxedPlane::BoxedPlane(const Vector4& a, const Vector4& b, const Vector4& c)
:	m_value(a, b, c)
{
}

BoxedPlane::BoxedPlane(float a, float b, float c, float d)
:	m_value(a, b, c, d)
{
}

Ref< BoxedVector4 > BoxedPlane::rayIntersection(
	const Vector4& origin,
	const Vector4& direction
) const
{
	Vector4 result;
	Scalar k;
	
	if (m_value.rayIntersection(origin, direction, k, &result))
		return new BoxedVector4(result);
	else
		return 0;
}

Ref< BoxedVector4 > BoxedPlane::segmentIntersection(
	const Vector4& a,
	const Vector4& b
) const
{
	Vector4 result;
	Scalar k;
	
	if (m_value.segmentIntersection(a, b, k, &result))
		return new BoxedVector4(result);
	else
		return 0;
}

Ref< BoxedVector4 > BoxedPlane::uniqueIntersectionPoint(
	const Plane& a,
	const Plane& b,
	const Plane& c
)
{
	Vector4 result;
	if (Plane::uniqueIntersectionPoint(a, b, c, result))
		return new BoxedVector4(result);
	else
		return 0;
}

std::wstring BoxedPlane::toString() const
{
	StringOutputStream ss;
	ss << m_value.normal() << L", " << m_value.distance();
	return ss.str();
}

void* BoxedPlane::operator new (size_t size)
{
	return s_allocBoxedPlane.alloc();
}

void BoxedPlane::operator delete (void* ptr)
{
	s_allocBoxedPlane.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedTransform, Boxed)

BoxedTransform::BoxedTransform()
{
}

BoxedTransform::BoxedTransform(const Transform& value)
:	m_value(value)
{
}

BoxedTransform::BoxedTransform(const BoxedVector4* translation, const BoxedQuaternion* rotation)
:	m_value(translation->unbox(), rotation->unbox())
{
}

BoxedTransform::BoxedTransform(const BoxedMatrix44* m)
:	m_value(m->unbox())
{
}

Plane BoxedTransform::get_planeX() const
{
	return Plane(m_value.axisX(), m_value.translation());
}

Plane BoxedTransform::get_planeY() const
{
	return Plane(m_value.axisY(), m_value.translation());
}

Plane BoxedTransform::get_planeZ() const
{
	return Plane(m_value.axisZ(), m_value.translation());
}

Transform BoxedTransform::inverse() const
{
	return m_value.inverse();
}

Matrix44 BoxedTransform::toMatrix44() const
{
	return m_value.toMatrix44();
}

Transform BoxedTransform::concat(const BoxedTransform* t) const
{
	return m_value * t->m_value;
}

Vector4 BoxedTransform::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

std::wstring BoxedTransform::toString() const
{
	return L"(transform)";
}

void* BoxedTransform::operator new (size_t size)
{
	return s_allocBoxedTransform.alloc();
}

void BoxedTransform::operator delete (void* ptr)
{
	s_allocBoxedTransform.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Aabb2", BoxedAabb2, Boxed)

BoxedAabb2::BoxedAabb2()
{
}

BoxedAabb2::BoxedAabb2(const Aabb2& value)
:	m_value(value)
{
}

BoxedAabb2::BoxedAabb2(const BoxedVector2* min, const BoxedVector2* max)
:	m_value(min->unbox(), max->unbox())
{
}

std::wstring BoxedAabb2::toString() const
{
	return L"(aabb2)";
}

void* BoxedAabb2::operator new (size_t size)
{
	return s_allocBoxedAabb2.alloc();
}

void BoxedAabb2::operator delete (void* ptr)
{
	s_allocBoxedAabb2.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Aabb3", BoxedAabb3, Boxed)

BoxedAabb3::BoxedAabb3()
{
}

BoxedAabb3::BoxedAabb3(const Aabb3& value)
:	m_value(value)
{
}

BoxedAabb3::BoxedAabb3(const BoxedVector4* min, const BoxedVector4* max)
:	m_value(min->unbox(), max->unbox())
{
}

Any BoxedAabb3::intersectRay(const BoxedVector4* origin, const BoxedVector4* direction) const
{
	Scalar distanceEnter, distanceExit;
	if (m_value.intersectRay(origin->unbox(), direction->unbox(), distanceEnter, distanceExit))
		return Any::fromFloat(distanceEnter);
	else
		return Any();
}

std::wstring BoxedAabb3::toString() const
{
	return L"(aabb3)";
}

void* BoxedAabb3::operator new (size_t size)
{
	return s_allocBoxedAabb3.alloc();
}

void BoxedAabb3::operator delete (void* ptr)
{
	s_allocBoxedAabb3.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Frustum", BoxedFrustum, Boxed)

BoxedFrustum::BoxedFrustum()
{
}

BoxedFrustum::BoxedFrustum(const Frustum& value)
:	m_value(value)
{
}

void BoxedFrustum::buildPerspective(float vfov, float aspect, float zn, float zf)
{
	m_value.buildPerspective(vfov, aspect, zn, zf);
}

void BoxedFrustum::buildOrtho(float width, float height, float zn, float zf)
{
	m_value.buildOrtho(width, height, zn, zf);
}

void BoxedFrustum::setNearZ(float zn)
{
	m_value.setNearZ(Scalar(zn));
}

float BoxedFrustum::getNearZ() const
{
	return m_value.getNearZ();
}

void BoxedFrustum::setFarZ(float zf)
{
	m_value.setFarZ(Scalar(zf));
}

float BoxedFrustum::getFarZ() const
{
	return m_value.getFarZ();
}

bool BoxedFrustum::insidePoint(const BoxedVector4* point) const
{
	return m_value.inside(point->unbox()) != Frustum::IrOutside;
}

int32_t BoxedFrustum::insideSphere(const BoxedVector4* center, float radius) const
{
	return m_value.inside(center->unbox(), Scalar(radius));
}

int32_t BoxedFrustum::insideAabb(const BoxedAabb3* aabb) const
{
	return m_value.inside(aabb->unbox());
}

const Plane& BoxedFrustum::getPlane(int32_t index) const
{
	return m_value.planes[index];
}

const Vector4& BoxedFrustum::getCorner(int32_t index) const
{
	return m_value.corners[index];
}

const Vector4& BoxedFrustum::getCenter() const
{
	return m_value.center;
}

std::wstring BoxedFrustum::toString() const
{
	return L"(frustum)";
}

void* BoxedFrustum::operator new (size_t size)
{
	return s_allocBoxedFrustum.alloc();
}

void BoxedFrustum::operator delete (void* ptr)
{
	s_allocBoxedFrustum.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Matrix33", BoxedMatrix33, Boxed)

BoxedMatrix33::BoxedMatrix33()
{
}

BoxedMatrix33::BoxedMatrix33(const Matrix33& value)
:	m_value(value)
{
}

Vector4 BoxedMatrix33::diagonal() const
{
	return m_value.diagonal();
}

float BoxedMatrix33::determinant() const
{
	return m_value.determinant();
}

Matrix33 BoxedMatrix33::transpose() const
{
	return m_value.transpose();
}

Matrix33 BoxedMatrix33::inverse() const
{
	return m_value.inverse();
}

void BoxedMatrix33::set(int r, int c, float v)
{
	m_value.e[r][c] = v;
}

float BoxedMatrix33::get(int r, int c) const
{
	return m_value.e[r][c];
}

Matrix33 BoxedMatrix33::concat(const BoxedMatrix33* t) const
{
	return m_value * t->unbox();
}

Vector2 BoxedMatrix33::transform(const BoxedVector2* v) const
{
	return m_value * v->unbox();
}

Matrix33 BoxedMatrix33::translate(float x, float y)
{
	return traktor::translate(x, y);
}

Matrix33 BoxedMatrix33::scale(float x, float y)
{
	return traktor::scale(x, y);
}

Matrix33 BoxedMatrix33::rotate(float angle)
{
	return traktor::rotate(angle);
}

std::wstring BoxedMatrix33::toString() const
{
	StringOutputStream ss;
	ss << m_value.e11 << L", " << m_value.e12 << L", " << m_value.e13 << Endl;
	ss << m_value.e21 << L", " << m_value.e22 << L", " << m_value.e23 << Endl;
	ss << m_value.e31 << L", " << m_value.e32 << L", " << m_value.e33 << Endl;
	return ss.str();
}

void* BoxedMatrix33::operator new (size_t size)
{
	return s_allocBoxedMatrix33.alloc();
}

void BoxedMatrix33::operator delete (void* ptr)
{
	return s_allocBoxedMatrix33.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Matrix44", BoxedMatrix44, Boxed)

BoxedMatrix44::BoxedMatrix44()
{
}

BoxedMatrix44::BoxedMatrix44(const Matrix44& value)
:	m_value(value)
{
}

BoxedMatrix44::BoxedMatrix44(const BoxedVector4* axisX, const BoxedVector4* axisY, const BoxedVector4* axisZ, const BoxedVector4* translation)
:	m_value(axisX->unbox(), axisY->unbox(), axisZ->unbox(), translation->unbox())
{
}

Vector4 BoxedMatrix44::axisX() const
{
	return m_value.axisX();
}

Vector4 BoxedMatrix44::axisY() const
{
	return m_value.axisY();
}

Vector4 BoxedMatrix44::axisZ() const
{
	return m_value.axisZ();
}

Plane BoxedMatrix44::planeX() const
{
	return Plane(m_value.axisX(), m_value.translation());
}

Plane BoxedMatrix44::planeY() const
{
	return Plane(m_value.axisY(), m_value.translation());
}

Plane BoxedMatrix44::planeZ() const
{
	return Plane(m_value.axisZ(), m_value.translation());
}

Vector4 BoxedMatrix44::translation() const
{
	return m_value.translation();
}

Vector4 BoxedMatrix44::diagonal() const
{
	return m_value.diagonal();
}

bool BoxedMatrix44::isOrtho() const
{
	return m_value.isOrtho();
}

float BoxedMatrix44::determinant() const
{
	return m_value.determinant();
}

Matrix44 BoxedMatrix44::transpose() const
{
	return m_value.transpose();
}

Matrix44 BoxedMatrix44::inverse() const
{
	return m_value.inverse();
}

void BoxedMatrix44::setColumn(int c, const BoxedVector4* v)
{
	m_value.set(c, v->unbox());
}

Vector4 BoxedMatrix44::getColumn(int c)
{
	return m_value.get(c);
}

void BoxedMatrix44::setRow(int r, const BoxedVector4* v)
{
	const Vector4& vv = v->unbox();
	m_value.set(r, 0, vv.x());
	m_value.set(r, 1, vv.x());
	m_value.set(r, 2, vv.x());
	m_value.set(r, 3, vv.x());
}

Vector4 BoxedMatrix44::getRow(int r)
{
	return Vector4(
		m_value.get(r, 0),
		m_value.get(r, 1),
		m_value.get(r, 2),
		m_value.get(r, 3)
	);
}

void BoxedMatrix44::set(int r, int c, float v)
{
	m_value.set(r, c, Scalar(v));
}

float BoxedMatrix44::get(int r, int c) const
{
	return m_value.get(r, c);
}

Matrix44 BoxedMatrix44::concat(const BoxedMatrix44* t) const
{
	return m_value * t->m_value;
}

Vector4 BoxedMatrix44::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

std::wstring BoxedMatrix44::toString() const
{
	StringOutputStream ss;
	ss << m_value(0, 0) << L", " << m_value(0, 1) << L", " << m_value(0, 2) << L", " << m_value(0, 3) << Endl;
	ss << m_value(1, 0) << L", " << m_value(1, 1) << L", " << m_value(1, 2) << L", " << m_value(1, 3) << Endl;
	ss << m_value(2, 0) << L", " << m_value(2, 1) << L", " << m_value(2, 2) << L", " << m_value(2, 3) << Endl;
	ss << m_value(3, 0) << L", " << m_value(3, 1) << L", " << m_value(3, 2) << L", " << m_value(3, 3) << Endl;
	return ss.str();
}

void* BoxedMatrix44::operator new (size_t size)
{
	return s_allocBoxedMatrix44.alloc();
}

void BoxedMatrix44::operator delete (void* ptr)
{
	s_allocBoxedMatrix44.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Color4f", BoxedColor4f, Boxed)

BoxedColor4f::BoxedColor4f(const Color4f& value)
:	m_value(value)
{
}

BoxedColor4f::BoxedColor4f()
:	m_value(1.0f, 1.0f, 1.0f, 1.0f)
{
}

BoxedColor4f::BoxedColor4f(float red, float green, float blue)
:	m_value(red, green, blue, 1.0f)
{
}

BoxedColor4f::BoxedColor4f(float red, float green, float blue, float alpha)
:	m_value(red, green, blue, alpha)
{
}

std::wstring BoxedColor4f::toString() const
{
	StringOutputStream ss;
	ss << m_value.getRed() << L", " << m_value.getGreen() << L", " << m_value.getBlue() << L", " << m_value.getAlpha();
	return ss.str();
}

void* BoxedColor4f::operator new (size_t size)
{
	return s_allocBoxedColor4f.alloc();
}

void BoxedColor4f::operator delete (void* ptr)
{
	s_allocBoxedColor4f.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Color4ub", BoxedColor4ub, Boxed)

BoxedColor4ub::BoxedColor4ub(const Color4ub& value)
:	m_value(value)
{
}

BoxedColor4ub::BoxedColor4ub()
:	m_value(255, 255, 255, 255)
{
}

BoxedColor4ub::BoxedColor4ub(uint8_t red, uint8_t green, uint8_t blue)
:	m_value(red, green, blue, 255)
{
}

BoxedColor4ub::BoxedColor4ub(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
:	m_value(red, green, blue, alpha)
{
}

std::wstring BoxedColor4ub::formatRGB() const
{
	StringOutputStream ss;
	FormatHex(ss, m_value.r, 2);
	FormatHex(ss, m_value.g, 2);
	FormatHex(ss, m_value.b, 2);
	return ss.str();
}

std::wstring BoxedColor4ub::formatARGB() const
{
	StringOutputStream ss;
	FormatHex(ss, m_value.r, 2);
	FormatHex(ss, m_value.g, 2);
	FormatHex(ss, m_value.b, 2);
	FormatHex(ss, m_value.a, 2);
	return ss.str();
}

std::wstring BoxedColor4ub::toString() const
{
	StringOutputStream ss;
	ss << int32_t(m_value.r) << L", " << int32_t(m_value.g) << L", " << int32_t(m_value.b) << L", " << int32_t(m_value.a);
	return ss.str();
}

void* BoxedColor4ub::operator new (size_t size)
{
	return s_allocBoxedColor4ub.alloc();
}

void BoxedColor4ub::operator delete (void* ptr)
{
	s_allocBoxedColor4ub.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Random", BoxedRandom, Boxed)

BoxedRandom::BoxedRandom()
{
}

BoxedRandom::BoxedRandom(const Random& value)
:	m_value(value)
{
}

BoxedRandom::BoxedRandom(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandom::toString() const
{
	return L"(random)";
}

void* BoxedRandom::operator new (size_t size)
{
	return s_allocBoxedRandom.alloc();
}

void BoxedRandom::operator delete (void* ptr)
{
	s_allocBoxedRandom.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.RandomGeometry", BoxedRandomGeometry, BoxedRandom)

BoxedRandomGeometry::BoxedRandomGeometry()
{
}

BoxedRandomGeometry::BoxedRandomGeometry(const RandomGeometry& value)
:	m_value(value)
{
}

BoxedRandomGeometry::BoxedRandomGeometry(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandomGeometry::toString() const
{
	return L"(random geometry)";
}

void* BoxedRandomGeometry::operator new (size_t size)
{
	return s_allocBoxedRandomGeometry.alloc();
}

void BoxedRandomGeometry::operator delete (void* ptr)
{
	s_allocBoxedRandomGeometry.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Ray3", BoxedRay3, Boxed)

BoxedRay3::BoxedRay3()
{
}

BoxedRay3::BoxedRay3(const Ray3& value)
:	m_value(value)
{
}

std::wstring BoxedRay3::toString() const
{
	StringOutputStream ss;
	ss << L"(" << m_value.origin << L") - (" << m_value.direction << L")";
	return ss.str();
}

void* BoxedRay3::operator new (size_t size)
{
	return s_allocBoxedRay3.alloc();
}

void BoxedRay3::operator delete (void* ptr)
{
	s_allocBoxedRay3.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.RefArray", BoxedRefArray, Boxed)

BoxedRefArray::BoxedRefArray()
{
}

int32_t BoxedRefArray::size() const
{
	return int32_t(m_arr.size());
}

void BoxedRefArray::set(int32_t index, ITypedObject* object)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = object;
}

ITypedObject* BoxedRefArray::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return 0;
}

void BoxedRefArray::push_back(ITypedObject* object)
{
	m_arr.push_back(object);
}

void BoxedRefArray::pop_back()
{
	m_arr.pop_back();
}

ITypedObject* BoxedRefArray::front()
{
	return m_arr.front();
}

ITypedObject* BoxedRefArray::back()
{
	return m_arr.back();
}

std::wstring BoxedRefArray::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedRefArray::operator new (size_t size)
{
	return s_allocBoxedRefArray.alloc();
}

void BoxedRefArray::operator delete (void* ptr)
{
	s_allocBoxedRefArray.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.Range", BoxedRange, Boxed)

BoxedRange::BoxedRange()
{
}

std::wstring BoxedRange::toString() const
{
	StringOutputStream ss;
	ss << m_min.getWideString() << L" - " << m_max.getWideString();
	return ss.str();
}

void* BoxedRange::operator new (size_t size)
{
	return s_allocBoxedRange.alloc();
}

void BoxedRange::operator delete (void* ptr)
{
	s_allocBoxedRange.free(ptr);
}


T_IMPLEMENT_RTTI_CLASS(L"traktor.StdVector", BoxedStdVector, Boxed)

BoxedStdVector::BoxedStdVector()
{
}

BoxedStdVector::BoxedStdVector(uint32_t size)
:	m_arr(size)
{
}

void BoxedStdVector::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedStdVector::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedStdVector::clear()
{
	m_arr.clear();
}

int32_t BoxedStdVector::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedStdVector::empty() const
{
	return m_arr.empty();
}

void BoxedStdVector::push_back(const Any& value)
{
	m_arr.push_back(value);
}

void BoxedStdVector::pop_back()
{
	m_arr.pop_back();
}

const Any& BoxedStdVector::front()
{
	return m_arr.front();
}

const Any& BoxedStdVector::back()
{
	return m_arr.back();
}

void BoxedStdVector::set(int32_t index, const Any& value)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = value;
}

Any BoxedStdVector::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return Any();
}

std::wstring BoxedStdVector::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedStdVector::operator new (size_t size)
{
	return s_allocBoxedStdVector.alloc();
}

void BoxedStdVector::operator delete (void* ptr)
{
	s_allocBoxedStdVector.free(ptr);
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.AlignedVector", BoxedAlignedVector, Boxed)

BoxedAlignedVector::BoxedAlignedVector()
{
}

BoxedAlignedVector::BoxedAlignedVector(uint32_t size)
:	m_arr(size)
{
}

void BoxedAlignedVector::reserve(uint32_t capacity)
{
	m_arr.reserve(capacity);
}

void BoxedAlignedVector::resize(uint32_t size)
{
	m_arr.resize(size);
}

void BoxedAlignedVector::clear()
{
	m_arr.clear();
}

int32_t BoxedAlignedVector::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedAlignedVector::empty() const
{
	return m_arr.empty();
}

void BoxedAlignedVector::push_back(const Any& value)
{
	m_arr.push_back(value);
}

void BoxedAlignedVector::pop_back()
{
	m_arr.pop_back();
}

const Any& BoxedAlignedVector::front()
{
	return m_arr.front();
}

const Any& BoxedAlignedVector::back()
{
	return m_arr.back();
}

void BoxedAlignedVector::set(int32_t index, const Any& value)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = value;
}

Any BoxedAlignedVector::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return Any();
}

std::wstring BoxedAlignedVector::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedAlignedVector::operator new (size_t size)
{
	return s_allocBoxedAlignedVector.alloc();
}

void BoxedAlignedVector::operator delete (void* ptr)
{
	s_allocBoxedAlignedVector.free(ptr);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.BoxesClassFactory", 0, BoxesClassFactory, IRuntimeClassFactory)

void BoxesClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Boxed > > classBoxed = new AutoRuntimeClass< Boxed >();
	classBoxed->addMethod("toString", &Boxed::toString);
	registrar->registerClass(classBoxed);

	Ref< AutoRuntimeClass< BoxedTypeInfo > > classBoxedTypeInfo = new AutoRuntimeClass< BoxedTypeInfo >();
	classBoxedTypeInfo->addProperty("name", &BoxedTypeInfo::getName);
	classBoxedTypeInfo->addProperty("size", &BoxedTypeInfo::getSize);
	classBoxedTypeInfo->addProperty("version", &BoxedTypeInfo::getVersion);
	classBoxedTypeInfo->addMethod("createInstance", &BoxedTypeInfo::createInstance);
	classBoxedTypeInfo->addStaticMethod("find", &BoxedTypeInfo::find);
	classBoxedTypeInfo->addStaticMethod("findAllOf", &BoxedTypeInfo::findAllOf);
	registrar->registerClass(classBoxedTypeInfo);

	Ref< AutoRuntimeClass< BoxedGuid > > classBoxedGuid = new AutoRuntimeClass< BoxedGuid >();
	classBoxedGuid->addConstructor();
	classBoxedGuid->addConstructor< const std::wstring& >();
	classBoxedGuid->addStaticMethod("create", &BoxedGuid::create);
	classBoxedGuid->addMethod("set", &BoxedGuid::set);
	classBoxedGuid->addMethod("format", &BoxedGuid::format);
	classBoxedGuid->addMethod("isValid", &BoxedGuid::isValid);
	classBoxedGuid->addMethod("isNull", &BoxedGuid::isNull);
	classBoxedGuid->addMethod("isNotNull", &BoxedGuid::isNotNull);
	registrar->registerClass(classBoxedGuid);

	Ref< AutoRuntimeClass< BoxedVector2 > > classBoxedVector2 = new AutoRuntimeClass< BoxedVector2 >();
	classBoxedVector2->addConstructor();
	classBoxedVector2->addConstructor< float, float >();
	classBoxedVector2->addConstant("zero", CastAny< Vector2 >::set(Vector2::zero()));
	classBoxedVector2->addProperty< float >("x", &BoxedVector2::set_x, &BoxedVector2::get_x);
	classBoxedVector2->addProperty< float >("y", &BoxedVector2::set_y, &BoxedVector2::get_y);
	classBoxedVector2->addProperty< float >("length", 0, &BoxedVector2::get_length);
	classBoxedVector2->addMethod("set", &BoxedVector2::set);
	classBoxedVector2->addMethod< Vector2, const BoxedVector2* >("add", &BoxedVector2::add);
	classBoxedVector2->addMethod< Vector2, const BoxedVector2* >("sub", &BoxedVector2::sub);
	classBoxedVector2->addMethod< Vector2, const BoxedVector2* >("mul", &BoxedVector2::mul);
	classBoxedVector2->addMethod< Vector2, const BoxedVector2* >("div", &BoxedVector2::div);
	classBoxedVector2->addMethod< Vector2, float >("addf", &BoxedVector2::add);
	classBoxedVector2->addMethod< Vector2, float >("subf", &BoxedVector2::sub);
	classBoxedVector2->addMethod< Vector2, float >("mulf", &BoxedVector2::mul);
	classBoxedVector2->addMethod< Vector2, float >("divf", &BoxedVector2::div);
	classBoxedVector2->addMethod("dot", &BoxedVector2::dot);
	classBoxedVector2->addMethod("normalized", &BoxedVector2::normalized);
	classBoxedVector2->addMethod("neg", &BoxedVector2::neg);
	classBoxedVector2->addMethod("perpendicular", &BoxedVector2::perpendicular);
	classBoxedVector2->addStaticMethod("lerp", &BoxedVector2::lerp);
	classBoxedVector2->addStaticMethod("distance", &BoxedVector2::distance);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, float >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, float >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, float >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('/', &BoxedVector2::div);
	classBoxedVector2->addOperator< Vector2, float >('/', &BoxedVector2::div);
	registrar->registerClass(classBoxedVector2);

	Ref< AutoRuntimeClass< BoxedVector4 > > classBoxedVector4 = new AutoRuntimeClass< BoxedVector4 >();
	classBoxedVector4->addConstructor();
	classBoxedVector4->addConstructor< float, float, float >();
	classBoxedVector4->addConstructor< float, float, float, float >();
	classBoxedVector4->addConstant("zero", CastAny< Vector4 >::set(Vector4::zero()));
	classBoxedVector4->addConstant("origo", CastAny< Vector4 >::set(Vector4::origo()));
	classBoxedVector4->addProperty< float >("x", &BoxedVector4::set_x, &BoxedVector4::get_x);
	classBoxedVector4->addProperty< float >("y", &BoxedVector4::set_y, &BoxedVector4::get_y);
	classBoxedVector4->addProperty< float >("z", &BoxedVector4::set_z, &BoxedVector4::get_z);
	classBoxedVector4->addProperty< float >("w", &BoxedVector4::set_w, &BoxedVector4::get_w);
	classBoxedVector4->addProperty< Vector4 >("xyz0", &BoxedVector4::get_xyz0);
	classBoxedVector4->addProperty< Vector4 >("xyz1", &BoxedVector4::get_xyz1);
	classBoxedVector4->addProperty< float >("length", &BoxedVector4::get_length);
	classBoxedVector4->addMethod("set", &BoxedVector4::set);
	classBoxedVector4->addMethod< Vector4, const BoxedVector4* >("add", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, const BoxedVector4* >("sub", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, const BoxedVector4* >("mul", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, const BoxedVector4* >("div", &BoxedVector4::div);
	classBoxedVector4->addMethod< Vector4, float >("addf", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, float >("subf", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, float >("mulf", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, float >("divf", &BoxedVector4::div);
	classBoxedVector4->addMethod("dot", &BoxedVector4::dot);
	classBoxedVector4->addMethod("cross", &BoxedVector4::cross);
	classBoxedVector4->addMethod("normalized", &BoxedVector4::normalized);
	classBoxedVector4->addMethod("neg", &BoxedVector4::neg);
	classBoxedVector4->addStaticMethod("lerp", &BoxedVector4::lerp);
	classBoxedVector4->addStaticMethod("distance3", &BoxedVector4::distance3);
	classBoxedVector4->addStaticMethod("distance4", &BoxedVector4::distance4);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, float >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, float >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, float >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('/', &BoxedVector4::div);
	classBoxedVector4->addOperator< Vector4, float >('/', &BoxedVector4::div);
	registrar->registerClass(classBoxedVector4);

	Ref< AutoRuntimeClass< BoxedVector4Array > > classBoxedVector4Array = new AutoRuntimeClass< BoxedVector4Array >();
	classBoxedVector4Array->addConstructor();
	classBoxedVector4Array->addConstructor< uint32_t >();
	classBoxedVector4Array->addMethod("reserve", &BoxedVector4Array::reserve);
	classBoxedVector4Array->addMethod("resize", &BoxedVector4Array::resize);
	classBoxedVector4Array->addMethod("clear", &BoxedVector4Array::clear);
	classBoxedVector4Array->addMethod("size", &BoxedVector4Array::size);
	classBoxedVector4Array->addMethod("push_back", &BoxedVector4Array::push_back);
	classBoxedVector4Array->addMethod("pop_back", &BoxedVector4Array::pop_back);
	classBoxedVector4Array->addMethod("front", &BoxedVector4Array::front);
	classBoxedVector4Array->addMethod("back", &BoxedVector4Array::back);
	classBoxedVector4Array->addMethod("set", &BoxedVector4Array::set);
	classBoxedVector4Array->addMethod("get", &BoxedVector4Array::get);
	registrar->registerClass(classBoxedVector4Array);

	Ref< AutoRuntimeClass< BoxedQuaternion > > classBoxedQuaternion = new AutoRuntimeClass< BoxedQuaternion >();
	classBoxedQuaternion->addConstructor();
	classBoxedQuaternion->addConstructor< float, float, float, float >();
	classBoxedQuaternion->addConstructor< const BoxedVector4*, float >();
	classBoxedQuaternion->addConstructor< float, float, float >();
	classBoxedQuaternion->addConstructor< const BoxedVector4*, const BoxedVector4* >();
	classBoxedQuaternion->addConstructor< const BoxedMatrix44* >();
	classBoxedQuaternion->addConstant("identity", CastAny< Quaternion >::set(Quaternion::identity()));
	classBoxedQuaternion->addProperty< float >("x", &BoxedQuaternion::set_x, &BoxedQuaternion::get_x);
	classBoxedQuaternion->addProperty< float >("y", &BoxedQuaternion::set_y, &BoxedQuaternion::get_y);
	classBoxedQuaternion->addProperty< float >("x", &BoxedQuaternion::set_z, &BoxedQuaternion::get_z);
	classBoxedQuaternion->addProperty< float >("y", &BoxedQuaternion::set_w, &BoxedQuaternion::get_w);
	classBoxedQuaternion->addMethod("normalized", &BoxedQuaternion::normalized);
	classBoxedQuaternion->addMethod("inverse", &BoxedQuaternion::inverse);
	classBoxedQuaternion->addMethod("concat", &BoxedQuaternion::concat);
	classBoxedQuaternion->addMethod("transform", &BoxedQuaternion::transform);
	classBoxedQuaternion->addMethod("getEulerAngles", &BoxedQuaternion::getEulerAngles);
	classBoxedQuaternion->addMethod("getAxisAngle", &BoxedQuaternion::getAxisAngle);
	classBoxedQuaternion->addStaticMethod("fromEulerAngles", &BoxedQuaternion::fromEulerAngles);
	classBoxedQuaternion->addStaticMethod("fromAxisAngle", &BoxedQuaternion::fromAxisAngle);
	classBoxedQuaternion->addStaticMethod("lerp", &BoxedQuaternion::lerp);
	classBoxedQuaternion->addStaticMethod("slerp", &BoxedQuaternion::slerp);
	classBoxedQuaternion->addOperator< Vector4, const BoxedVector4* >('*', &BoxedQuaternion::transform);
	classBoxedQuaternion->addOperator< Quaternion, const BoxedQuaternion* >('*', &BoxedQuaternion::concat);
	registrar->registerClass(classBoxedQuaternion);

	Ref< AutoRuntimeClass< BoxedPlane > > classBoxedPlane = new AutoRuntimeClass< BoxedPlane >();
	classBoxedPlane->addConstructor();
	classBoxedPlane->addConstructor< const Plane& >();
	classBoxedPlane->addConstructor< const Vector4&, float >();
	classBoxedPlane->addConstructor< const Vector4&, const Vector4& >();
	classBoxedPlane->addConstructor< const Vector4&, const Vector4&, const Vector4& >();
	classBoxedPlane->addConstructor< float, float, float, float >();
	classBoxedPlane->addMethod("setNormal", &BoxedPlane::setNormal);
	classBoxedPlane->addMethod("setDistance", &BoxedPlane::setDistance);
	classBoxedPlane->addMethod("normal", &BoxedPlane::normal);
	classBoxedPlane->addMethod("distance", &BoxedPlane::distance);
	classBoxedPlane->addMethod("distanceToPoint", &BoxedPlane::distanceToPoint);
	classBoxedPlane->addMethod("project", &BoxedPlane::project);
	classBoxedPlane->addMethod("rayIntersection", &BoxedPlane::rayIntersection);
	classBoxedPlane->addMethod("segmentIntersection", &BoxedPlane::segmentIntersection);
	classBoxedPlane->addStaticMethod("uniqueIntersectionPoint", &BoxedPlane::uniqueIntersectionPoint);
	registrar->registerClass(classBoxedPlane);

	Ref< AutoRuntimeClass< BoxedTransform > > classBoxedTransform = new AutoRuntimeClass< BoxedTransform >();
	classBoxedTransform->addConstructor();
	classBoxedTransform->addConstructor< const BoxedVector4*, const BoxedQuaternion* >();
	classBoxedTransform->addConstructor< const BoxedMatrix44* >();
	classBoxedTransform->addConstant("identity", CastAny< Transform >::set(Transform::identity()));
	classBoxedTransform->addProperty< const Vector4& >("translation", 0, &BoxedTransform::get_translation);
	classBoxedTransform->addProperty< const Quaternion& >("rotation", 0, &BoxedTransform::get_rotation);
	classBoxedTransform->addProperty< Vector4 >("axisX", &BoxedTransform::get_axisX);
	classBoxedTransform->addProperty< Vector4 >("axisY", &BoxedTransform::get_axisY);
	classBoxedTransform->addProperty< Vector4 >("axisZ", &BoxedTransform::get_axisZ);
	classBoxedTransform->addProperty< Plane >("planeX", &BoxedTransform::get_planeX);
	classBoxedTransform->addProperty< Plane >("planeY", &BoxedTransform::get_planeY);
	classBoxedTransform->addProperty< Plane >("planeZ", &BoxedTransform::get_planeZ);
	classBoxedTransform->addMethod("inverse", &BoxedTransform::inverse);
	classBoxedTransform->addMethod("toMatrix44", &BoxedTransform::toMatrix44);
	classBoxedTransform->addMethod("concat", &BoxedTransform::concat);
	classBoxedTransform->addMethod("transform", &BoxedTransform::transform);
	classBoxedTransform->addStaticMethod("lerp", &BoxedTransform::lerp);
	classBoxedTransform->addOperator< Vector4, const BoxedVector4* >('*', &BoxedTransform::transform);
	classBoxedTransform->addOperator< Transform, const BoxedTransform* >('*', &BoxedTransform::concat);
	registrar->registerClass(classBoxedTransform);

	Ref< AutoRuntimeClass< BoxedAabb2 > > classBoxedAabb2 = new AutoRuntimeClass< BoxedAabb2 >();
	classBoxedAabb2->addConstructor();
	classBoxedAabb2->addConstructor< const BoxedVector2*, const BoxedVector2* >();
	classBoxedAabb2->addMethod("min", &BoxedAabb2::min);
	classBoxedAabb2->addMethod("max", &BoxedAabb2::max);
	classBoxedAabb2->addMethod("inside", &BoxedAabb2::inside);
	classBoxedAabb2->addMethod("contain", &BoxedAabb2::contain);
	classBoxedAabb2->addMethod("getCenter", &BoxedAabb2::getCenter);
	classBoxedAabb2->addMethod("getExtent", &BoxedAabb2::getExtent);
	classBoxedAabb2->addMethod("empty", &BoxedAabb2::empty);
	registrar->registerClass(classBoxedAabb2);

	Ref< AutoRuntimeClass< BoxedAabb3 > > classBoxedAabb3 = new AutoRuntimeClass< BoxedAabb3 >();
	classBoxedAabb3->addConstructor();
	classBoxedAabb3->addConstructor< const BoxedVector4*, const BoxedVector4* >();
	classBoxedAabb3->addMethod("min", &BoxedAabb3::min);
	classBoxedAabb3->addMethod("max", &BoxedAabb3::max);
	classBoxedAabb3->addMethod("inside", &BoxedAabb3::inside);
	classBoxedAabb3->addMethod("contain", &BoxedAabb3::contain);
	classBoxedAabb3->addMethod("scale", &BoxedAabb3::scale);
	classBoxedAabb3->addMethod("expand", &BoxedAabb3::expand);
	classBoxedAabb3->addMethod("transform", &BoxedAabb3::transform);
	classBoxedAabb3->addMethod("getMin", &BoxedAabb3::getMin);
	classBoxedAabb3->addMethod("getMax", &BoxedAabb3::getMax);
	classBoxedAabb3->addMethod("getCenter", &BoxedAabb3::getCenter);
	classBoxedAabb3->addMethod("getExtent", &BoxedAabb3::getExtent);
	classBoxedAabb3->addMethod("empty", &BoxedAabb3::empty);
	classBoxedAabb3->addMethod("overlap", &BoxedAabb3::overlap);
	classBoxedAabb3->addMethod("intersectRay", &BoxedAabb3::intersectRay);
	registrar->registerClass(classBoxedAabb3);

	Ref< AutoRuntimeClass< BoxedFrustum > > classBoxedFrustum = new AutoRuntimeClass< BoxedFrustum >();
	classBoxedFrustum->addConstructor();
	classBoxedFrustum->addMethod("buildPerspective", &BoxedFrustum::buildPerspective);
	classBoxedFrustum->addMethod("buildOrtho", &BoxedFrustum::buildOrtho);
	classBoxedFrustum->addMethod("setNearZ", &BoxedFrustum::setNearZ);
	classBoxedFrustum->addMethod("getNearZ", &BoxedFrustum::getNearZ);
	classBoxedFrustum->addMethod("setFarZ", &BoxedFrustum::setFarZ);
	classBoxedFrustum->addMethod("getFarZ", &BoxedFrustum::getFarZ);
	classBoxedFrustum->addMethod("insidePoint", &BoxedFrustum::insidePoint);
	classBoxedFrustum->addMethod("insideSphere", &BoxedFrustum::insideSphere);
	classBoxedFrustum->addMethod("insideAabb", &BoxedFrustum::insideAabb);
	classBoxedFrustum->addMethod("getPlane", &BoxedFrustum::getPlane);
	classBoxedFrustum->addMethod("getCorner", &BoxedFrustum::getCorner);
	classBoxedFrustum->addMethod("getCenter", &BoxedFrustum::getCenter);
	registrar->registerClass(classBoxedFrustum);

	Ref< AutoRuntimeClass< BoxedMatrix33 > > classBoxedMatrix33 = new AutoRuntimeClass< BoxedMatrix33 >();
	classBoxedMatrix33->addConstructor();
	classBoxedMatrix33->addConstant("zero", CastAny< Matrix33 >::set(Matrix33::zero()));
	classBoxedMatrix33->addConstant("identity", CastAny< Matrix33 >::set(Matrix33::identity()));
	classBoxedMatrix33->addMethod("diagonal", &BoxedMatrix33::diagonal);
	classBoxedMatrix33->addMethod("determinant", &BoxedMatrix33::determinant);
	classBoxedMatrix33->addMethod("transpose", &BoxedMatrix33::transpose);
	classBoxedMatrix33->addMethod("inverse", &BoxedMatrix33::inverse);
	classBoxedMatrix33->addMethod("set", &BoxedMatrix33::set);
	classBoxedMatrix33->addMethod("get", &BoxedMatrix33::get);
	classBoxedMatrix33->addMethod("concat", &BoxedMatrix33::concat);
	classBoxedMatrix33->addMethod("transform", &BoxedMatrix33::transform);
	classBoxedMatrix33->addStaticMethod("translate", &BoxedMatrix33::translate);
	classBoxedMatrix33->addStaticMethod("scale", &BoxedMatrix33::scale);
	classBoxedMatrix33->addStaticMethod("rotate", &BoxedMatrix33::rotate);
	classBoxedMatrix33->addOperator< Vector2, const BoxedVector2* >('*', &BoxedMatrix33::transform);
	classBoxedMatrix33->addOperator< Matrix33, const BoxedMatrix33* >('*', &BoxedMatrix33::concat);
	registrar->registerClass(classBoxedMatrix33);

	Ref< AutoRuntimeClass< BoxedMatrix44 > > classBoxedMatrix44 = new AutoRuntimeClass< BoxedMatrix44 >();
	classBoxedMatrix44->addConstructor();
	classBoxedMatrix44->addConstructor< const BoxedVector4*, const BoxedVector4*, const BoxedVector4*, const BoxedVector4* >();
	classBoxedMatrix44->addConstant("zero", CastAny< Matrix44 >::set(Matrix44::zero()));
	classBoxedMatrix44->addConstant("identity", CastAny< Matrix44 >::set(Matrix44::identity()));
	classBoxedMatrix44->addMethod("axisX", &BoxedMatrix44::axisX);
	classBoxedMatrix44->addMethod("axisY", &BoxedMatrix44::axisY);
	classBoxedMatrix44->addMethod("axisZ", &BoxedMatrix44::axisZ);
	classBoxedMatrix44->addMethod("planeX", &BoxedMatrix44::planeX);
	classBoxedMatrix44->addMethod("planeY", &BoxedMatrix44::planeY);
	classBoxedMatrix44->addMethod("planeZ", &BoxedMatrix44::planeZ);
	classBoxedMatrix44->addMethod("translation", &BoxedMatrix44::translation);
	classBoxedMatrix44->addMethod("diagonal", &BoxedMatrix44::diagonal);
	classBoxedMatrix44->addMethod("isOrtho", &BoxedMatrix44::isOrtho);
	classBoxedMatrix44->addMethod("determinant", &BoxedMatrix44::determinant);
	classBoxedMatrix44->addMethod("transpose", &BoxedMatrix44::transpose);
	classBoxedMatrix44->addMethod("inverse", &BoxedMatrix44::inverse);
	classBoxedMatrix44->addMethod("setColumn", &BoxedMatrix44::setColumn);
	classBoxedMatrix44->addMethod("getColumn", &BoxedMatrix44::getColumn);
	classBoxedMatrix44->addMethod("setRow", &BoxedMatrix44::setRow);
	classBoxedMatrix44->addMethod("getRow", &BoxedMatrix44::getRow);
	classBoxedMatrix44->addMethod("set", &BoxedMatrix44::set);
	classBoxedMatrix44->addMethod("get", &BoxedMatrix44::get);
	classBoxedMatrix44->addMethod("concat", &BoxedMatrix44::concat);
	classBoxedMatrix44->addMethod("transform", &BoxedMatrix44::transform);
	classBoxedMatrix44->addOperator< Vector4, const BoxedVector4* >('*', &BoxedMatrix44::transform);
	classBoxedMatrix44->addOperator< Matrix44, const BoxedMatrix44* >('*', &BoxedMatrix44::concat);
	registrar->registerClass(classBoxedMatrix44);

	Ref< AutoRuntimeClass< BoxedColor4f > > classBoxedColor4f = new AutoRuntimeClass< BoxedColor4f >();
	classBoxedColor4f->addConstructor();
	classBoxedColor4f->addConstructor< float, float, float >();
	classBoxedColor4f->addConstructor< float, float, float, float >();
	classBoxedColor4f->addMethod("get", &BoxedColor4f::get);
	classBoxedColor4f->addMethod("getRed", &BoxedColor4f::getRed);
	classBoxedColor4f->addMethod("getGreen", &BoxedColor4f::getGreen);
	classBoxedColor4f->addMethod("getBlue", &BoxedColor4f::getBlue);
	classBoxedColor4f->addMethod("getAlpha", &BoxedColor4f::getAlpha);
	classBoxedColor4f->addMethod("set", &BoxedColor4f::set);
	classBoxedColor4f->addMethod("setRed", &BoxedColor4f::setRed);
	classBoxedColor4f->addMethod("setGreen", &BoxedColor4f::setGreen);
	classBoxedColor4f->addMethod("setBlue", &BoxedColor4f::setBlue);
	classBoxedColor4f->addMethod("setAlpha", &BoxedColor4f::setAlpha);
	registrar->registerClass(classBoxedColor4f);

	Ref< AutoRuntimeClass< BoxedColor4ub > > classBoxedColor4ub = new AutoRuntimeClass< BoxedColor4ub >();
	classBoxedColor4ub->addConstructor();
	classBoxedColor4ub->addConstructor< uint8_t, uint8_t, uint8_t >();
	classBoxedColor4ub->addConstructor< uint8_t, uint8_t, uint8_t, uint8_t >();
	classBoxedColor4ub->addMethod("getRed", &BoxedColor4ub::getRed);
	classBoxedColor4ub->addMethod("getGreen", &BoxedColor4ub::getGreen);
	classBoxedColor4ub->addMethod("getBlue", &BoxedColor4ub::getBlue);
	classBoxedColor4ub->addMethod("getAlpha", &BoxedColor4ub::getAlpha);
	classBoxedColor4ub->addMethod("getARGB", &BoxedColor4ub::getARGB);
	classBoxedColor4ub->addMethod("setRed", &BoxedColor4ub::setRed);
	classBoxedColor4ub->addMethod("setGreen", &BoxedColor4ub::setGreen);
	classBoxedColor4ub->addMethod("setBlue", &BoxedColor4ub::setBlue);
	classBoxedColor4ub->addMethod("setAlpha", &BoxedColor4ub::setAlpha);
	classBoxedColor4ub->addMethod("formatRGB", &BoxedColor4ub::formatRGB);
	classBoxedColor4ub->addMethod("formatARGB", &BoxedColor4ub::formatARGB);
	registrar->registerClass(classBoxedColor4ub);

	Ref< AutoRuntimeClass< BoxedRandom > > classBoxedRandom = new AutoRuntimeClass< BoxedRandom >();
	classBoxedRandom->addConstructor();
	classBoxedRandom->addConstructor< uint32_t >();
	classBoxedRandom->addMethod("next", &BoxedRandom::next);
	classBoxedRandom->addMethod("nextFloat", &BoxedRandom::nextFloat);
	registrar->registerClass(classBoxedRandom);

	Ref< AutoRuntimeClass< BoxedRandomGeometry > > classBoxedRandomGeometry = new AutoRuntimeClass< BoxedRandomGeometry >();
	classBoxedRandomGeometry->addConstructor();
	classBoxedRandomGeometry->addConstructor< uint32_t >();
	classBoxedRandomGeometry->addMethod("nextUnit", &BoxedRandomGeometry::nextUnit);
	classBoxedRandomGeometry->addMethod("nextHemi", &BoxedRandomGeometry::nextHemi);
	registrar->registerClass(classBoxedRandomGeometry);

	Ref< AutoRuntimeClass< BoxedRay3 > > classBoxedRay3 = new AutoRuntimeClass< BoxedRay3 >();
	classBoxedRay3->addConstructor();
	classBoxedRay3->addConstructor< const Ray3& >();
	classBoxedRay3->addMethod("origin", &BoxedRay3::origin);
	classBoxedRay3->addMethod("direction", &BoxedRay3::direction);
	classBoxedRay3->addMethod("distance", &BoxedRay3::distance);
	classBoxedRay3->addOperator< Vector4, float >('*', &BoxedRay3::mul);
	registrar->registerClass(classBoxedRay3);

	Ref< AutoRuntimeClass< BoxedRange > > classBoxedRange = new AutoRuntimeClass< BoxedRange >();
	classBoxedRange->addConstructor();
	classBoxedRange->addMethod("min", &BoxedRange::min);
	classBoxedRange->addMethod("max", &BoxedRange::max);
	registrar->registerClass(classBoxedRange);

	Ref< AutoRuntimeClass< BoxedRefArray > > classBoxedRefArray = new AutoRuntimeClass< BoxedRefArray >();
	classBoxedRefArray->addConstructor();
	classBoxedRefArray->addConstructor< const RefArray< Object >& >();
	classBoxedRefArray->addMethod("size", &BoxedRefArray::size);
	classBoxedRefArray->addMethod("set", &BoxedRefArray::set);
	classBoxedRefArray->addMethod("get", &BoxedRefArray::get);
	classBoxedRefArray->addMethod("push_back", &BoxedRefArray::push_back);
	classBoxedRefArray->addMethod("pop_back", &BoxedRefArray::pop_back);
	classBoxedRefArray->addMethod("front", &BoxedRefArray::front);
	classBoxedRefArray->addMethod("back", &BoxedRefArray::back);
	classBoxedRefArray->addMethod("pushBack", &BoxedRefArray::push_back);	// \deprecated
	classBoxedRefArray->addMethod("popBack", &BoxedRefArray::pop_back);		// \deprecated
	registrar->registerClass(classBoxedRefArray);

	Ref< AutoRuntimeClass< BoxedStdVector > > classBoxedStdVector = new AutoRuntimeClass< BoxedStdVector >();
	classBoxedStdVector->addConstructor();
	classBoxedStdVector->addConstructor< uint32_t >();
	classBoxedStdVector->addMethod("reserve", &BoxedStdVector::reserve);
	classBoxedStdVector->addMethod("resize", &BoxedStdVector::resize);
	classBoxedStdVector->addMethod("clear", &BoxedStdVector::clear);
	classBoxedStdVector->addMethod("size", &BoxedStdVector::size);
	classBoxedStdVector->addMethod("empty", &BoxedStdVector::empty);
	classBoxedStdVector->addMethod("push_back", &BoxedStdVector::push_back);
	classBoxedStdVector->addMethod("pop_back", &BoxedStdVector::pop_back);
	classBoxedStdVector->addMethod("front", &BoxedStdVector::front);
	classBoxedStdVector->addMethod("back", &BoxedStdVector::back);
	classBoxedStdVector->addMethod("set", &BoxedStdVector::set);
	classBoxedStdVector->addMethod("get", &BoxedStdVector::get);
	registrar->registerClass(classBoxedStdVector);

	Ref< AutoRuntimeClass< BoxedAlignedVector > > classBoxedAlignedVector = new AutoRuntimeClass< BoxedAlignedVector >();
	classBoxedAlignedVector->addConstructor();
	classBoxedAlignedVector->addConstructor< uint32_t >();
	classBoxedAlignedVector->addMethod("reserve", &BoxedAlignedVector::reserve);
	classBoxedAlignedVector->addMethod("resize", &BoxedAlignedVector::resize);
	classBoxedAlignedVector->addMethod("clear", &BoxedAlignedVector::clear);
	classBoxedAlignedVector->addMethod("size", &BoxedAlignedVector::size);
	classBoxedAlignedVector->addMethod("empty", &BoxedAlignedVector::empty);
	classBoxedAlignedVector->addMethod("push_back", &BoxedAlignedVector::push_back);
	classBoxedAlignedVector->addMethod("pop_back", &BoxedAlignedVector::pop_back);
	classBoxedAlignedVector->addMethod("front", &BoxedAlignedVector::front);
	classBoxedAlignedVector->addMethod("back", &BoxedAlignedVector::back);
	classBoxedAlignedVector->addMethod("set", &BoxedAlignedVector::set);
	classBoxedAlignedVector->addMethod("get", &BoxedAlignedVector::get);
	registrar->registerClass(classBoxedAlignedVector);
}

}
