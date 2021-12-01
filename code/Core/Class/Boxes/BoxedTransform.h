#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedMatrix44;
class BoxedQuaternion;
class BoxedVector4;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedTransform : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedTransform() = default;

	explicit BoxedTransform(const Transform& value) : m_value(value) {}

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

	static Transform lookAt(const BoxedVector4* position, const BoxedVector4* target, const BoxedVector4* up);

	const Transform& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Transform m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Transform, false >
{
	static std::wstring typeName() {
		return L"traktor.Transform";
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

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Transform&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Transform&";
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

}
