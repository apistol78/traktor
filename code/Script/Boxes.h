#ifndef traktor_script_Boxes_H
#define traktor_script_Boxes_H

#include "Core/Object.h"
#include "Core/RefArray.h"
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
	
class T_DLLCLASS BoxedVector4 : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedVector4();
	
	explicit BoxedVector4(const Vector4& value);

	explicit BoxedVector4(float x, float y, float z);
	
	explicit BoxedVector4(float x, float y, float z, float w);

	void set(float x, float y, float z, float w);
	
	float x() const;
	
	float y() const;
	
	float z() const;
	
	float w() const;

	float length() const;

	Vector4 normalized() const;
	
	const Vector4& unbox() const {
		return m_value;
	}
	
private:
	Vector4 m_value;
};

class T_DLLCLASS BoxedQuaternion : public Object
{
	T_RTTI_CLASS;
	
public:
	BoxedQuaternion();
	
	explicit BoxedQuaternion(const Quaternion& value);
	
	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const Vector4& axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const Vector4& from, const Vector4& to);
	
	float x() const;
	
	float y() const;
	
	float z() const;
	
	float w() const;

	Quaternion normalized() const;

	Quaternion inverse() const;
	
	const Quaternion& unbox() const;

private:
	Quaternion m_value;
};

class T_DLLCLASS BoxedTransform : public Object
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

class T_DLLCLASS BoxedArray : public Object
{
	T_RTTI_CLASS;

public:
	BoxedArray();

	template < typename ObjectType >
	BoxedArray(const RefArray< ObjectType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = arr[i];
	}

	int32_t length() const;

	void set(int32_t index, Object* object);

	Object* get(int32_t index);

	const RefArray< Object >& unbox() const;

private:
	RefArray< Object > m_arr;
};

void T_DLLCLASS registerBoxClasses(class IScriptManager* scriptManager);
	
	}
}

#endif	// traktor_script_Boxes_H
