#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
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

	static int32_t minorAxis3(const BoxedVector4* a) { return traktor::minorAxis3(a->m_value); }

	static int32_t majorAxis3(const BoxedVector4* a) { return traktor::majorAxis3(a->m_value); }

	const Vector4& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Vector4 m_value;
};

/*!
 * \ingroup Core
 */
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

/*!
 * \ingroup Core
 */
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

}
