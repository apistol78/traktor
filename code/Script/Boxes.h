#ifndef traktor_script_Boxes_H
#define traktor_script_Boxes_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"

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
	
class BoxedVector4 : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedVector4();
	
	explicit BoxedVector4(const Vector4& value);

	explicit BoxedVector4(float x, float y, float z);
	
	explicit BoxedVector4(float x, float y, float z, float w);
	
	float x() const;
	
	float y() const;
	
	float z() const;
	
	float w() const;
	
	const Vector4& unbox() const {
		return m_value;
	}
	
private:
	Vector4 m_value;
};

class BoxedQuaternion : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);
	
	float x() const;
	
	float y() const;
	
	float z() const;
	
	float w() const;
	
	const Quaternion& unbox() const;

private:
	Quaternion m_value;
};

class BoxedTransform : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedTransform();
	
	explicit BoxedTransform(const Transform& value);
	
	explicit BoxedTransform(const Vector4& translation, const Quaternion& rotation);
	
	const Vector4& translation() const;
	
	const Quaternion& rotation() const;
	
	const Transform& unbox() const;

private:
	Transform m_value;
};

void T_DLLCLASS registerBoxClasses(class IScriptManager* scriptManager);
	
	}
}

#endif	// traktor_script_Boxes_H
