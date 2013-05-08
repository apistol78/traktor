#ifndef traktor_BinarySerializer_H
#define traktor_BinarySerializer_H

#include <map>
#include "Core/Ref.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Binary serializer.
 * \ingroup Core
 */
class T_DLLCLASS BinarySerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	BinarySerializer(IStream* stream);
	
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

	virtual void operator >> (const Member< Color4ub >& m);

	virtual void operator >> (const Member< Color4f >& m);

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
	Ref< IStream > m_stream;
	Direction m_direction;
	std::map< uint64_t, Ref< ISerializable > > m_readCache;
	std::map< ISerializable*, uint64_t > m_writeCache;
	uint64_t m_nextCacheId;
	std::vector< const TypeInfo*> m_typeReadCache;
	std::map< const TypeInfo*, uint32_t > m_typeWriteCache;
	uint32_t m_nextTypeCacheId;
};
	
}

#endif	// traktor_BinarySerializer_H
