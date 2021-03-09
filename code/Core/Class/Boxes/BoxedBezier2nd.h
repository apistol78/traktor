#pragma once

#include "Core/RefArray.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Math/Bezier2nd.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedBezier3rd;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedBezier2nd : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedBezier2nd() = default;

	explicit BoxedBezier2nd(const Bezier2nd& value);

	explicit BoxedBezier2nd(const BoxedVector2* cp0, const BoxedVector2* cp1, const BoxedVector2* cp2);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float getLocalMinMaxY() const;

	float getLocalMinMaxX() const;

	//void intersectX(float y, float& outT0, float& outT1) const;

	//void intersectY(float x, float& outT0, float& outT1) const;

	RefArray< BoxedBezier2nd > split(float t) const;

	Ref< BoxedBezier3rd > toBezier3rd() const;

	const Vector2& cp0() const { return m_value.cp0; }

	const Vector2& cp1() const { return m_value.cp1; }

	const Vector2& cp2() const { return m_value.cp2; }

	const Bezier2nd& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

private:
	Bezier2nd m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Bezier2nd, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Bezier2nd";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier2nd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier2nd& value) {
		return Any::fromObject(new BoxedBezier2nd(value));
	}
	static const Bezier2nd& get(const Any& value) {
		return static_cast< BoxedBezier2nd* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Bezier2nd&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Bezier2nd&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedBezier2nd >(value.getObjectUnsafe());
	}
	static Any set(const Bezier2nd& value) {
		return Any::fromObject(new BoxedBezier2nd(value));
	}
	static const Bezier2nd& get(const Any& value) {
		return static_cast< BoxedBezier2nd* >(value.getObject())->unbox();
	}
};

}
