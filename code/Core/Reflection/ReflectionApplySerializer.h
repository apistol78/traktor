#ifndef traktor_ReflectionApplySerializer_H
#define traktor_ReflectionApplySerializer_H

#include "Core/RefArray.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

class ReflectionMember;
class RfmCompound;

/*! \brief Internal reflection apply serializer.
 * \ingroup Core
 */
class ReflectionApplySerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	ReflectionApplySerializer(const RfmCompound* compound);

	virtual Direction getDirection() const;

	virtual void operator >> (const Member< bool >& m);

	virtual void operator >> (const Member< int8_t >& m);

	virtual void operator >> (const Member< uint8_t >& m);

	virtual void operator >> (const Member< int16_t >& m);

	virtual void operator >> (const Member< uint16_t >& m);

	virtual void operator >> (const Member< int32_t >& m);

	virtual void operator >> (const Member< uint32_t >& m);

	virtual void operator >> (const Member< int64_t >& m);

	virtual void operator >> (const Member< uint64_t >& m);

	virtual void operator >> (const Member< float >& m);

	virtual void operator >> (const Member< double >& m);

	virtual void operator >> (const Member< std::string >& m);

	virtual void operator >> (const Member< std::wstring >& m);

	virtual void operator >> (const Member< Guid >& m);

	virtual void operator >> (const Member< Path >& m);

	virtual void operator >> (const Member< Color4f >& m);

	virtual void operator >> (const Member< Color4ub >& m);

	virtual void operator >> (const Member< Scalar >& m);

	virtual void operator >> (const Member< Vector2 >& m);

	virtual void operator >> (const Member< Vector4 >& m);

	virtual void operator >> (const Member< Matrix33 >& m);

	virtual void operator >> (const Member< Matrix44 >& m);

	virtual void operator >> (const Member< Quaternion >& m);

	virtual void operator >> (const Member< ISerializable* >& m);

	virtual void operator >> (const Member< void* >& m);

	virtual void operator >> (const MemberArray& m);

	virtual void operator >> (const MemberComplex& m);

	virtual void operator >> (const MemberEnumBase& m);

private:
	friend class Reflection;

	Ref< const RfmCompound > m_compoundMember;
	uint32_t m_memberIndex;

	Ref< const ReflectionMember > getNextMember();
};

}

#endif	// traktor_ReflectionApplySerializer_H
