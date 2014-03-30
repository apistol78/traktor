#ifndef traktor_RfmObject_H
#define traktor_RfmObject_H

#include "Core/Ref.h"
#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! \brief Reflected object member.
 * \ingroup Core
 */
class T_DLLCLASS RfmObject : public ReflectionMember
{
	T_RTTI_CLASS;

public:
	RfmObject(const wchar_t* name, ISerializable* value);

	void set(ISerializable* value) { m_value = value; }

	ISerializable* get() const { return m_value; }

	virtual bool replace(const ReflectionMember* source);

private:
	Ref< ISerializable > m_value;
};

}

#endif	// traktor_RfmObject_H
