#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Script/AutoScriptClass.h"
#include "Script/CastAny.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Boxed", Boxed, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.UInt64", BoxedUInt64, Boxed)

BoxedUInt64::BoxedUInt64()
{
}

BoxedUInt64::BoxedUInt64(uint64_t value)
:	m_value(value)
{
}

std::wstring BoxedUInt64::format() const
{
	return traktor::toString(m_value);
}

std::wstring BoxedUInt64::toString() const
{
	return format();
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

Vector2 BoxedVector2::add(const Vector2& v) const
{
	return m_value + v;
}

Vector2 BoxedVector2::sub(const Vector2& v) const
{
	return m_value - v;
}

Vector2 BoxedVector2::mul(const Vector2& v) const
{
	return m_value * v;
}

Vector2 BoxedVector2::div(const Vector2& v) const
{
	return m_value / v;
}

Vector2 BoxedVector2::add(float v) const
{
	return m_value + v;
}

Vector2 BoxedVector2::sub(float v) const
{
	return m_value - v;
}

Vector2 BoxedVector2::mul(float v) const
{
	return m_value * v;
}

Vector2 BoxedVector2::div(float v) const
{
	return m_value / v;
}

float BoxedVector2::dot(const Vector2& v) const
{
	return traktor::dot(m_value, v);
}

float BoxedVector2::length() const
{
	return m_value.length();
}

Vector2 BoxedVector2::normalized() const
{
	return m_value.normalized();
}

Vector2 BoxedVector2::neg() const
{
	return -m_value;
}

Vector2 BoxedVector2::perpendicular() const
{
	return m_value.perpendicular();
}

std::wstring BoxedVector2::toString() const
{
	StringOutputStream ss;
	ss << m_value.x << L", " << m_value.y;
	return ss.str();
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

void BoxedVector4::set(float x, float y, float z, float w)
{
	m_value.set(x, y, z, w);
}

Vector4 BoxedVector4::add(const Vector4& v) const
{
	return m_value + v;
}

Vector4 BoxedVector4::sub(const Vector4& v) const
{
	return m_value - v;
}

Vector4 BoxedVector4::mul(const Vector4& v) const
{
	return m_value * v;
}

Vector4 BoxedVector4::div(const Vector4& v) const
{
	return m_value / v;
}

Vector4 BoxedVector4::add(float v) const
{
	return m_value + Scalar(v);
}

Vector4 BoxedVector4::sub(float v) const
{
	return m_value - Scalar(v);
}

Vector4 BoxedVector4::mul(float v) const
{
	return m_value * Scalar(v);
}

Vector4 BoxedVector4::div(float v) const
{
	return m_value / Scalar(v);
}

float BoxedVector4::dot(const Vector4& v) const
{
	return dot3(m_value, v);
}

Vector4 BoxedVector4::cross(const Vector4& v) const
{
	return traktor::cross(m_value, v);
}

float BoxedVector4::length() const
{
	return m_value.length();
}

Vector4 BoxedVector4::normalized() const
{
	return m_value.normalized();
}

Vector4 BoxedVector4::neg() const
{
	return -m_value;
}

std::wstring BoxedVector4::toString() const
{
	StringOutputStream ss;
	ss << m_value.x() << L", " << m_value.y() << L", " << m_value.z() << L", " << m_value.w();
	return ss.str();
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

BoxedQuaternion::BoxedQuaternion(const Vector4& axis, float angle)
{
	m_value = Quaternion::fromAxisAngle(axis, Scalar(angle));
}

BoxedQuaternion::BoxedQuaternion(float head, float pitch, float bank)
{
	m_value = Quaternion::fromEulerAngles(head, pitch, bank);
}

BoxedQuaternion::BoxedQuaternion(const Vector4& from, const Vector4& to)
:	m_value(from, to)
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

Quaternion BoxedQuaternion::concat(const Quaternion& q) const
{
	return m_value * q;
}

Vector4 BoxedQuaternion::transform(const Vector4& v) const
{
	return m_value * v;
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

Quaternion BoxedQuaternion::fromAxisAngle(const Vector4& axisAngle)
{
	return Quaternion::fromAxisAngle(axisAngle);
}

std::wstring BoxedQuaternion::toString() const
{
	StringOutputStream ss;
	ss << m_value.e.x() << L", " << m_value.e.y() << L", " << m_value.e.z() << L", " << m_value.e.w();
	return ss.str();
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

std::wstring BoxedPlane::toString() const
{
	StringOutputStream ss;
	ss << m_value.normal().x() << L", " << m_value.normal().y() << L", " << m_value.normal().z() << L", " << m_value.distance();
	return ss.str();
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedTransform, Boxed)

BoxedTransform::BoxedTransform()
{
}

BoxedTransform::BoxedTransform(const Transform& value)
:	m_value(value)
{
}

BoxedTransform::BoxedTransform(const Vector4& translation, const Quaternion& rotation)
:	m_value(translation, rotation)
{
}

const Vector4& BoxedTransform::translation() const
{
	return m_value.translation();
}

const Quaternion& BoxedTransform::rotation() const
{
	return m_value.rotation();
}

Vector4 BoxedTransform::axisX() const
{
	return m_value.axisX();
}

Vector4 BoxedTransform::axisY() const
{
	return m_value.axisY();
}

Vector4 BoxedTransform::axisZ() const
{
	return m_value.axisZ();
}

Plane BoxedTransform::planeX() const
{
	return Plane(m_value.axisX(), m_value.translation());
}

Plane BoxedTransform::planeY() const
{
	return Plane(m_value.axisY(), m_value.translation());
}

Plane BoxedTransform::planeZ() const
{
	return Plane(m_value.axisZ(), m_value.translation());
}

Transform BoxedTransform::inverse() const
{
	return m_value.inverse();
}

Transform BoxedTransform::concat(const Transform& t) const
{
	return m_value * t;
}

Vector4 BoxedTransform::transform(const Vector4& v) const
{
	return m_value * v;
}

std::wstring BoxedTransform::toString() const
{
	return L"(transform)";
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Aabb3", BoxedAabb3, Boxed)

BoxedAabb3::BoxedAabb3()
{
}

BoxedAabb3::BoxedAabb3(const Aabb3& value)
:	m_value(value)
{
}

BoxedAabb3::BoxedAabb3(const Vector4& min, const Vector4& max)
:	m_value(min, max)
{
}

Any BoxedAabb3::intersectRay(const Vector4& origin, const Vector4& direction) const
{
	Scalar distanceEnter, distanceExit;
	if (m_value.intersectRay(origin, direction, distanceEnter, distanceExit))
		return Any::fromFloat(distanceEnter);
	else
		return Any();
}

std::wstring BoxedAabb3::toString() const
{
	return L"(aabb3)";
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

bool BoxedFrustum::insidePoint(const Vector4& point) const
{
	return m_value.inside(point) != Frustum::IrOutside;
}

int32_t BoxedFrustum::insideSphere(const Vector4& center, float radius) const
{
	return m_value.inside(center, Scalar(radius));
}

int32_t BoxedFrustum::insideAabb(const Aabb3& aabb) const
{
	return m_value.inside(aabb);
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

std::wstring BoxedColor4ub::toString() const
{
	StringOutputStream ss;
	ss << int32_t(m_value.r) << L", " << int32_t(m_value.g) << L", " << int32_t(m_value.b) << L", " << int32_t(m_value.a);
	return ss.str();
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Array", BoxedRefArray, Boxed)

BoxedRefArray::BoxedRefArray()
{
}

int32_t BoxedRefArray::size() const
{
	return int32_t(m_arr.size());
}

void BoxedRefArray::set(int32_t index, Object* object)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = object;
}

Object* BoxedRefArray::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return 0;
}

void BoxedRefArray::pushBack(Object* object)
{
	m_arr.push_back(object);
}

void BoxedRefArray::popBack()
{
	m_arr.pop_back();
}

Object* BoxedRefArray::front()
{
	return m_arr.front();
}

Object* BoxedRefArray::back()
{
	return m_arr.back();
}

std::wstring BoxedRefArray::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.StdVector", BoxedStdVector, Boxed)

BoxedStdVector::BoxedStdVector()
{
}

int32_t BoxedStdVector::size() const
{
	return int32_t(m_arr.size());
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

void registerBoxClasses(IScriptManager* scriptManager)
{
	Ref< AutoScriptClass< Boxed > > classBoxed = new AutoScriptClass< Boxed >();
	classBoxed->addMethod("toString", &Boxed::toString);
	scriptManager->registerClass(classBoxed);

	Ref< AutoScriptClass< BoxedUInt64 > > classBoxedUInt64 = new AutoScriptClass< BoxedUInt64 >();
	classBoxedUInt64->addConstructor();
	classBoxedUInt64->addMethod("format", &BoxedUInt64::format);
	scriptManager->registerClass(classBoxedUInt64);

	Ref< AutoScriptClass< BoxedGuid > > classBoxedGuid = new AutoScriptClass< BoxedGuid >();
	classBoxedGuid->addConstructor();
	classBoxedGuid->addConstructor< const std::wstring& >();
	classBoxedGuid->addStaticMethod("create", &BoxedGuid::create);
	classBoxedGuid->addMethod("set", &BoxedGuid::set);
	classBoxedGuid->addMethod("format", &BoxedGuid::format);
	classBoxedGuid->addMethod("isValid", &BoxedGuid::isValid);
	classBoxedGuid->addMethod("isNull", &BoxedGuid::isNull);
	classBoxedGuid->addMethod("isNotNull", &BoxedGuid::isNotNull);
	scriptManager->registerClass(classBoxedGuid);

	Ref< AutoScriptClass< BoxedVector2 > > classBoxedVector2 = new AutoScriptClass< BoxedVector2 >();
	classBoxedVector2->addConstructor();
	classBoxedVector2->addConstructor< float, float >();
	classBoxedVector2->addMethod("set", &BoxedVector2::set);
	classBoxedVector2->addMethod("x", &BoxedVector2::x);
	classBoxedVector2->addMethod("y", &BoxedVector2::y);
	classBoxedVector2->addMethod< Vector2, const Vector2& >("add", &BoxedVector2::add);
	classBoxedVector2->addMethod< Vector2, const Vector2& >("sub", &BoxedVector2::sub);
	classBoxedVector2->addMethod< Vector2, const Vector2& >("mul", &BoxedVector2::mul);
	classBoxedVector2->addMethod< Vector2, const Vector2& >("div", &BoxedVector2::div);
	classBoxedVector2->addMethod< Vector2, float >("addf", &BoxedVector2::add);
	classBoxedVector2->addMethod< Vector2, float >("subf", &BoxedVector2::sub);
	classBoxedVector2->addMethod< Vector2, float >("mulf", &BoxedVector2::mul);
	classBoxedVector2->addMethod< Vector2, float >("divf", &BoxedVector2::div);
	classBoxedVector2->addMethod("dot", &BoxedVector2::dot);
	classBoxedVector2->addMethod("length", &BoxedVector2::length);
	classBoxedVector2->addMethod("normalized", &BoxedVector2::normalized);
	classBoxedVector2->addMethod("neg", &BoxedVector2::neg);
	classBoxedVector2->addMethod("perpendicular", &BoxedVector2::perpendicular);
	classBoxedVector2->addStaticMethod("zero", &BoxedVector2::zero);
	classBoxedVector2->addStaticMethod("lerp", &BoxedVector2::lerp);
	classBoxedVector2->addOperator< Vector2, const Vector2& >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, float >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, const Vector2& >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, float >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, const Vector2& >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, float >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, const Vector2& >('/', &BoxedVector2::div);
	classBoxedVector2->addOperator< Vector2, float >('/', &BoxedVector2::div);
	scriptManager->registerClass(classBoxedVector2);

	Ref< AutoScriptClass< BoxedVector4 > > classBoxedVector4 = new AutoScriptClass< BoxedVector4 >();
	classBoxedVector4->addConstructor();
	classBoxedVector4->addConstructor< float, float, float >();
	classBoxedVector4->addConstructor< float, float, float, float >();
	classBoxedVector4->addMethod("set", &BoxedVector4::set);
	classBoxedVector4->addMethod("x", &BoxedVector4::x);
	classBoxedVector4->addMethod("y", &BoxedVector4::y);
	classBoxedVector4->addMethod("z", &BoxedVector4::z);
	classBoxedVector4->addMethod("w", &BoxedVector4::w);
	classBoxedVector4->addMethod("xyz0", &BoxedVector4::xyz0);
	classBoxedVector4->addMethod("xyz1", &BoxedVector4::xyz1);
	classBoxedVector4->addMethod< Vector4, const Vector4& >("add", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, const Vector4& >("sub", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, const Vector4& >("mul", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, const Vector4& >("div", &BoxedVector4::div);
	classBoxedVector4->addMethod< Vector4, float >("addf", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, float >("subf", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, float >("mulf", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, float >("divf", &BoxedVector4::div);
	classBoxedVector4->addMethod("dot", &BoxedVector4::dot);
	classBoxedVector4->addMethod("cross", &BoxedVector4::cross);
	classBoxedVector4->addMethod("length", &BoxedVector4::length);
	classBoxedVector4->addMethod("normalized", &BoxedVector4::normalized);
	classBoxedVector4->addMethod("neg", &BoxedVector4::neg);
	classBoxedVector4->addStaticMethod("zero", &BoxedVector4::zero);
	classBoxedVector4->addStaticMethod("origo", &BoxedVector4::origo);
	classBoxedVector4->addStaticMethod("lerp", &BoxedVector4::lerp);
	classBoxedVector4->addOperator< Vector4, const Vector4& >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, float >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, const Vector4& >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, float >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, const Vector4& >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, float >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, const Vector4& >('/', &BoxedVector4::div);
	classBoxedVector4->addOperator< Vector4, float >('/', &BoxedVector4::div);
	scriptManager->registerClass(classBoxedVector4);

	Ref< AutoScriptClass< BoxedQuaternion > > classBoxedQuaternion = new AutoScriptClass< BoxedQuaternion >();
	classBoxedQuaternion->addConstructor();
	classBoxedQuaternion->addConstructor< float, float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, float >();
	classBoxedQuaternion->addConstructor< float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, const Vector4& >();
	classBoxedQuaternion->addMethod("x", &BoxedQuaternion::x);
	classBoxedQuaternion->addMethod("y", &BoxedQuaternion::y);
	classBoxedQuaternion->addMethod("z", &BoxedQuaternion::z);
	classBoxedQuaternion->addMethod("w", &BoxedQuaternion::w);
	classBoxedQuaternion->addMethod("normalized", &BoxedQuaternion::normalized);
	classBoxedQuaternion->addMethod("inverse", &BoxedQuaternion::inverse);
	classBoxedQuaternion->addMethod("concat", &BoxedQuaternion::concat);
	classBoxedQuaternion->addMethod("transform", &BoxedQuaternion::transform);
	classBoxedQuaternion->addMethod("getEulerAngles", &BoxedQuaternion::getEulerAngles);
	classBoxedQuaternion->addMethod("getAxisAngle", &BoxedQuaternion::getAxisAngle);
	classBoxedQuaternion->addStaticMethod("identity", &BoxedQuaternion::identity);
	classBoxedQuaternion->addStaticMethod("fromEulerAngles", &BoxedQuaternion::fromEulerAngles);
	classBoxedQuaternion->addStaticMethod("fromAxisAngle", &BoxedQuaternion::fromAxisAngle);
	classBoxedQuaternion->addStaticMethod("lerp", &BoxedQuaternion::lerp);
	classBoxedQuaternion->addOperator< Vector4, const Vector4& >('*', &BoxedQuaternion::transform);
	classBoxedQuaternion->addOperator< Quaternion, const Quaternion& >('*', &BoxedQuaternion::concat);
	scriptManager->registerClass(classBoxedQuaternion);

	Ref< AutoScriptClass< BoxedPlane > > classBoxedPlane = new AutoScriptClass< BoxedPlane >();
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
	scriptManager->registerClass(classBoxedPlane);
	
	Ref< AutoScriptClass< BoxedTransform > > classBoxedTransform = new AutoScriptClass< BoxedTransform >();
	classBoxedTransform->addConstructor();
	classBoxedTransform->addConstructor< const Vector4&, const Quaternion& >();
	classBoxedTransform->addMethod("translation", &BoxedTransform::translation);
	classBoxedTransform->addMethod("rotation", &BoxedTransform::rotation);
	classBoxedTransform->addMethod("axisX", &BoxedTransform::axisX);
	classBoxedTransform->addMethod("axisY", &BoxedTransform::axisY);
	classBoxedTransform->addMethod("axisZ", &BoxedTransform::axisZ);
	classBoxedTransform->addMethod("planeX", &BoxedTransform::planeX);
	classBoxedTransform->addMethod("planeY", &BoxedTransform::planeY);
	classBoxedTransform->addMethod("planeZ", &BoxedTransform::planeZ);
	classBoxedTransform->addMethod("inverse", &BoxedTransform::inverse);
	classBoxedTransform->addMethod("concat", &BoxedTransform::concat);
	classBoxedTransform->addMethod("transform", &BoxedTransform::transform);
	classBoxedTransform->addStaticMethod("identity", &BoxedTransform::identity);
	classBoxedTransform->addStaticMethod("lerp", &BoxedTransform::lerp);
	classBoxedTransform->addOperator< Vector4, const Vector4& >('*', &BoxedTransform::transform);
	classBoxedTransform->addOperator< Transform, const Transform& >('*', &BoxedTransform::concat);
	scriptManager->registerClass(classBoxedTransform);

	Ref< AutoScriptClass< BoxedAabb3 > > classBoxedAabb3 = new AutoScriptClass< BoxedAabb3 >();
	classBoxedAabb3->addConstructor();
	classBoxedAabb3->addConstructor< const Vector4&, const Vector4& >();
	classBoxedAabb3->addMethod("inside", &BoxedAabb3::inside);
	classBoxedAabb3->addMethod("transform", &BoxedAabb3::transform);
	classBoxedAabb3->addMethod("getMin", &BoxedAabb3::getMin);
	classBoxedAabb3->addMethod("getMax", &BoxedAabb3::getMax);
	classBoxedAabb3->addMethod("getCenter", &BoxedAabb3::getCenter);
	classBoxedAabb3->addMethod("getExtent", &BoxedAabb3::getExtent);
	classBoxedAabb3->addMethod("empty", &BoxedAabb3::empty);
	classBoxedAabb3->addMethod("overlap", &BoxedAabb3::overlap);
	classBoxedAabb3->addMethod("intersectRay", &BoxedAabb3::intersectRay);
	scriptManager->registerClass(classBoxedAabb3);

	Ref< AutoScriptClass< BoxedFrustum > > classBoxedFrustum = new AutoScriptClass< BoxedFrustum >();
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
	scriptManager->registerClass(classBoxedFrustum);

	Ref< AutoScriptClass< BoxedColor4f > > classBoxedColor4f = new AutoScriptClass< BoxedColor4f >();
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
	scriptManager->registerClass(classBoxedColor4f);

	Ref< AutoScriptClass< BoxedColor4ub > > classBoxedColor4ub = new AutoScriptClass< BoxedColor4ub >();
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
	scriptManager->registerClass(classBoxedColor4ub);

	Ref< AutoScriptClass< BoxedRange > > classBoxedRange = new AutoScriptClass< BoxedRange >();
	classBoxedRange->addConstructor();
	classBoxedRange->addMethod("min", &BoxedRange::min);
	classBoxedRange->addMethod("max", &BoxedRange::max);
	scriptManager->registerClass(classBoxedRange);

	Ref< AutoScriptClass< BoxedRefArray > > classBoxedRefArray = new AutoScriptClass< BoxedRefArray >();
	classBoxedRefArray->addConstructor();
	classBoxedRefArray->addConstructor< const RefArray< Object >& >();
	classBoxedRefArray->addMethod("size", &BoxedRefArray::size);
	classBoxedRefArray->addMethod("set", &BoxedRefArray::set);
	classBoxedRefArray->addMethod("get", &BoxedRefArray::get);
	classBoxedRefArray->addMethod("pushBack", &BoxedRefArray::pushBack);
	classBoxedRefArray->addMethod("popBack", &BoxedRefArray::popBack);
	classBoxedRefArray->addMethod("front", &BoxedRefArray::front);
	classBoxedRefArray->addMethod("back", &BoxedRefArray::back);
	scriptManager->registerClass(classBoxedRefArray);

	Ref< AutoScriptClass< BoxedStdVector > > classBoxedStdVector = new AutoScriptClass< BoxedStdVector >();
	classBoxedStdVector->addConstructor();
	classBoxedStdVector->addMethod("size", &BoxedStdVector::size);
	classBoxedStdVector->addMethod("set", &BoxedStdVector::set);
	classBoxedStdVector->addMethod("get", &BoxedStdVector::get);
	scriptManager->registerClass(classBoxedStdVector);
}
	
	}
}
