/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
