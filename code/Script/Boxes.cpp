#include "Script/AutoScriptClass.h"
#include "Script/CastAny.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector2", BoxedVector2, Object)

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4", BoxedVector4, Object)

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.Quaternion", BoxedQuaternion, Object)

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
:	m_value(axis, angle)
{
}

BoxedQuaternion::BoxedQuaternion(float head, float pitch, float bank)
:	m_value(head, pitch, bank)
{
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedTransform, Object)

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.Array", BoxedRefArray, Object)

BoxedRefArray::BoxedRefArray()
{
}

int32_t BoxedRefArray::length() const
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.StdVector", BoxedStdVector, Object)

BoxedStdVector::BoxedStdVector()
{
}

int32_t BoxedStdVector::length() const
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

void registerBoxClasses(IScriptManager* scriptManager)
{
	Ref< AutoScriptClass< BoxedVector2 > > classBoxedVector2 = new AutoScriptClass< BoxedVector2 >();
	classBoxedVector2->addConstructor();
	classBoxedVector2->addConstructor< float, float >();
	classBoxedVector2->addMethod(L"set", &BoxedVector2::set);
	classBoxedVector2->addMethod(L"x", &BoxedVector2::x);
	classBoxedVector2->addMethod(L"y", &BoxedVector2::y);
	scriptManager->registerClass(classBoxedVector2);

	Ref< AutoScriptClass< BoxedVector4 > > classBoxedVector4 = new AutoScriptClass< BoxedVector4 >();
	classBoxedVector4->addConstructor();
	classBoxedVector4->addConstructor< float, float, float >();
	classBoxedVector4->addConstructor< float, float, float, float >();
	classBoxedVector4->addMethod(L"set", &BoxedVector4::set);
	classBoxedVector4->addMethod(L"x", &BoxedVector4::x);
	classBoxedVector4->addMethod(L"y", &BoxedVector4::y);
	classBoxedVector4->addMethod(L"z", &BoxedVector4::z);
	classBoxedVector4->addMethod(L"w", &BoxedVector4::w);
	classBoxedVector4->addMethod(L"xyz0", &BoxedVector4::xyz0);
	classBoxedVector4->addMethod(L"xyz1", &BoxedVector4::xyz1);
	classBoxedVector4->addMethod< Vector4, const Vector4& >(L"add", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, const Vector4& >(L"sub", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, const Vector4& >(L"mul", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, const Vector4& >(L"div", &BoxedVector4::div);
	classBoxedVector4->addMethod< Vector4, float >(L"addf", &BoxedVector4::add);
	classBoxedVector4->addMethod< Vector4, float >(L"subf", &BoxedVector4::sub);
	classBoxedVector4->addMethod< Vector4, float >(L"mulf", &BoxedVector4::mul);
	classBoxedVector4->addMethod< Vector4, float >(L"divf", &BoxedVector4::div);
	classBoxedVector4->addMethod(L"dot", &BoxedVector4::dot);
	classBoxedVector4->addMethod(L"length", &BoxedVector4::length);
	classBoxedVector4->addMethod(L"normalized", &BoxedVector4::normalized);
	classBoxedVector4->addMethod(L"neg", &BoxedVector4::neg);
	scriptManager->registerClass(classBoxedVector4);

	Ref< AutoScriptClass< BoxedQuaternion > > classBoxedQuaternion = new AutoScriptClass< BoxedQuaternion >();
	classBoxedQuaternion->addConstructor();
	classBoxedQuaternion->addConstructor< float, float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, float >();
	classBoxedQuaternion->addConstructor< float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, const Vector4& >();
	classBoxedQuaternion->addMethod(L"x", &BoxedQuaternion::x);
	classBoxedQuaternion->addMethod(L"y", &BoxedQuaternion::y);
	classBoxedQuaternion->addMethod(L"z", &BoxedQuaternion::z);
	classBoxedQuaternion->addMethod(L"w", &BoxedQuaternion::w);
	classBoxedQuaternion->addMethod(L"normalized", &BoxedQuaternion::normalized);
	classBoxedQuaternion->addMethod(L"inverse", &BoxedQuaternion::inverse);
	classBoxedQuaternion->addMethod(L"concat", &BoxedQuaternion::concat);
	classBoxedQuaternion->addMethod(L"transform", &BoxedQuaternion::transform);
	classBoxedQuaternion->addMethod(L"getEulerAngles", &BoxedQuaternion::getEulerAngles);
	classBoxedQuaternion->addMethod(L"getAxisAngle", &BoxedQuaternion::getAxisAngle);
	scriptManager->registerClass(classBoxedQuaternion);
	
	Ref< AutoScriptClass< BoxedTransform > > classBoxedTransform = new AutoScriptClass< BoxedTransform >();
	classBoxedTransform->addConstructor();
	classBoxedTransform->addConstructor< const Vector4&, const Quaternion& >();
	classBoxedTransform->addMethod(L"translation", &BoxedTransform::translation);
	classBoxedTransform->addMethod(L"rotation", &BoxedTransform::rotation);
	classBoxedTransform->addMethod(L"axisX", &BoxedTransform::axisX);
	classBoxedTransform->addMethod(L"axisY", &BoxedTransform::axisY);
	classBoxedTransform->addMethod(L"axisZ", &BoxedTransform::axisZ);
	classBoxedTransform->addMethod(L"inverse", &BoxedTransform::inverse);
	classBoxedTransform->addMethod(L"concat", &BoxedTransform::concat);
	classBoxedTransform->addMethod(L"transform", &BoxedTransform::transform);
	scriptManager->registerClass(classBoxedTransform);

	Ref< AutoScriptClass< BoxedRefArray > > classBoxedRefArray = new AutoScriptClass< BoxedRefArray >();
	classBoxedRefArray->addConstructor();
	classBoxedRefArray->addConstructor< const RefArray< Object >& >();
	classBoxedRefArray->addMethod(L"length", &BoxedRefArray::length);
	classBoxedRefArray->addMethod(L"set", &BoxedRefArray::set);
	classBoxedRefArray->addMethod(L"get", &BoxedRefArray::get);
	scriptManager->registerClass(classBoxedRefArray);

	Ref< AutoScriptClass< BoxedStdVector > > classBoxedStdVector = new AutoScriptClass< BoxedStdVector >();
	classBoxedStdVector->addConstructor();
	classBoxedStdVector->addMethod(L"length", &BoxedStdVector::length);
	classBoxedStdVector->addMethod(L"set", &BoxedStdVector::set);
	classBoxedStdVector->addMethod(L"get", &BoxedStdVector::get);
	scriptManager->registerClass(classBoxedStdVector);
}
	
	}
}
