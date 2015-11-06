#ifndef traktor_ReflectionInspectSerializer_H
#define traktor_ReflectionInspectSerializer_H

#include "Core/RefArray.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

class ReflectionMember;
class RfmCompound;

/*! \brief Internal reflection inspection serializer.
 * \ingroup Core
 */
class ReflectionInspectSerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	ReflectionInspectSerializer(RfmCompound* compound);

	virtual Direction getDirection() const T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< bool >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< int8_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< uint8_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< int16_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< uint16_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< int32_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< uint32_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< int64_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< uint64_t >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< float >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< double >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< std::string >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< std::wstring >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Guid >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Path >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Color4ub >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Color4f >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Scalar >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Vector2 >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Vector4 >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Matrix33 >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Matrix44 >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< Quaternion >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< ISerializable* >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const Member< void* >& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const MemberArray& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const MemberComplex& m) T_OVERRIDE T_FINAL;

	virtual void operator >> (const MemberEnumBase& m) T_OVERRIDE T_FINAL;

private:
	friend class Reflection;

	Ref< RfmCompound > m_compoundMember;

	bool addMember(ReflectionMember* member);
};

}

#endif	// traktor_ReflectionInspectSerializer_H
