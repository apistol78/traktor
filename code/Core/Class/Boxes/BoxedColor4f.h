#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Color4f.h"

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

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Color4f m_value;
};

/*!
 * \ingroup Core
 */
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

/*!
 * \ingroup Core
 */
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

}
