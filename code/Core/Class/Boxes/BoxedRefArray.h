#pragma once

#include "Core/RefArray.h"
#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"

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
class T_DLLCLASS BoxedRefArray : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRefArray();

	template < typename ObjectType >
	BoxedRefArray(const RefArray< ObjectType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = arr[i];
	}

	int32_t size() const;

	void set(int32_t index, ITypedObject* object);

	ITypedObject* get(int32_t index);

	void push_back(ITypedObject* object);

	void pop_back();

	ITypedObject* front();

	ITypedObject* back();

	virtual std::wstring toString() const override final;

	template < typename ObjectType >
	RefArray< ObjectType > unbox() const
	{
		RefArray< ObjectType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = mandatory_non_null_type_cast< ObjectType* >(m_arr[i]);
		return arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RefArray< ITypedObject > m_arr;
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny < RefArray< InnerType >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.RefArray< " << type_name< InnerType >() << L" >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
		return static_cast< BoxedRefArray* >(value.getObject())->unbox< InnerType >();
    }
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny < const RefArray< InnerType >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.RefArray< " << type_name< InnerType >() << L" >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRefArray >(value.getObjectUnsafe());
	}
    static Any set(const RefArray< InnerType >& value) {
        return Any::fromObject(new BoxedRefArray(value));
    }
    static RefArray< InnerType > get(const Any& value) {
		return static_cast< BoxedRefArray* >(value.getObject())->unbox< InnerType >();
    }
};

}
