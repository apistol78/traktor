#ifndef traktor_ISerializer_H
#define traktor_ISerializer_H

#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Object.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Serializer interface.
 * \ingroup Core
 */
class T_DLLCLASS ISerializer : public Object
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		SdRead,
		SdWrite
	};

	virtual int getVersion() const = 0;

	virtual Direction getDirection() const = 0;

	virtual ISerializable* getCurrentObject() = 0;

	virtual void failure() = 0;

	virtual void operator >> (const Member< bool >& m) = 0;

	virtual void operator >> (const Member< int8_t >& m) = 0;

	virtual void operator >> (const Member< uint8_t >& m) = 0;

	virtual void operator >> (const Member< int16_t >& m) = 0;

	virtual void operator >> (const Member< uint16_t >& m) = 0;

	virtual void operator >> (const Member< int32_t >& m) = 0;

	virtual void operator >> (const Member< uint32_t >& m) = 0;

	virtual void operator >> (const Member< int64_t >& m) = 0;

	virtual void operator >> (const Member< uint64_t >& m) = 0;

	virtual void operator >> (const Member< float >& m) = 0;

	virtual void operator >> (const Member< double >& m) = 0;

	virtual void operator >> (const Member< std::string >& m) = 0;

	virtual void operator >> (const Member< std::wstring >& m) = 0;

	virtual void operator >> (const Member< Guid >& m) = 0;

	virtual void operator >> (const Member< Path >& m) = 0;

	virtual void operator >> (const Member< Color4ub >& m) = 0;

	virtual void operator >> (const Member< Color4f >& m) = 0;

	virtual void operator >> (const Member< Scalar >& m) = 0;

	virtual void operator >> (const Member< Vector2 >& m) = 0;

	virtual void operator >> (const Member< Vector4 >& m) = 0;

	virtual void operator >> (const Member< Matrix33 >& m) = 0;

	virtual void operator >> (const Member< Matrix44 >& m) = 0;

	virtual void operator >> (const Member< Quaternion >& m) = 0;

	virtual void operator >> (const Member< ISerializable* >& m) = 0;

	virtual void operator >> (const Member< void* >& m) = 0;

	virtual void operator >> (const MemberArray& m) = 0;

	virtual void operator >> (const MemberComplex& m) = 0;

	virtual void operator >> (const MemberEnumBase& m) = 0;

	template < typename T >
	T* getCurrentObject()
	{
		return dynamic_type_cast< T* >(getCurrentObject());
	}

	bool ensure(bool condition)
	{
		if (!condition)
			failure();
		return condition;
	}
};

}

#endif	// traktor_ISerializer_H
