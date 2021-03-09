#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
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

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedVector2 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedVector2() = default;

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

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Vector2 m_value = Vector2(0.0f, 0.0f);
};

/*!
 * \ingroup Core
 */
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

/*!
 * \ingroup Core
 */
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

}
