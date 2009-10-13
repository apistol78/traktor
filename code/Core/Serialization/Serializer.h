#ifndef traktor_Serializer_H
#define traktor_Serializer_H

#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Serialization/Member.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;
class MemberArray;
class MemberComplex;
class MemberEnumBase;

/*! \brief Serializer base class.
 * \ingroup Core
 */
class T_DLLCLASS Serializer : public Object
{
	T_RTTI_CLASS(Serializer)

public:
	enum Direction
	{
		SdRead,
		SdWrite
	};

	Serializable* readObject();

	bool writeObject(const Serializable* o);

	Serializable* getCurrentObject();

	Serializable* getOuterObject();

	/*!
	 * \name Bidirectional interface.
	 * All of these methods are bidirectional, meaning
	 * they are used both for serialization and de-serialization transparently.
	 */
	//@{

	/*!
	 * Return version of document being serialized.
	 *
	 * \return Version of current object.
	 */
	int getVersion() const;

	virtual Direction getDirection() = 0;

	virtual bool operator >> (const Member< bool >& m) = 0;
	
	virtual bool operator >> (const Member< int8_t >& m) = 0;
	
	virtual bool operator >> (const Member< uint8_t >& m) = 0;
	
	virtual bool operator >> (const Member< int16_t >& m) = 0;
	
	virtual bool operator >> (const Member< uint16_t >& m) = 0;
	
	virtual bool operator >> (const Member< int32_t >& m) = 0;
	
	virtual bool operator >> (const Member< uint32_t >& m) = 0;

	virtual bool operator >> (const Member< int64_t >& m) = 0;

	virtual bool operator >> (const Member< uint64_t >& m) = 0;
	
	virtual bool operator >> (const Member< float >& m) = 0;
	
	virtual bool operator >> (const Member< double >& m) = 0;
	
	virtual bool operator >> (const Member< std::string >& m) = 0;

	virtual bool operator >> (const Member< std::wstring >& m) = 0;

	virtual bool operator >> (const Member< Guid >& m) = 0;

	virtual bool operator >> (const Member< Path >& m) = 0;

	virtual bool operator >> (const Member< Color >& m) = 0;

	virtual bool operator >> (const Member< Scalar >& m) = 0;
	
	virtual bool operator >> (const Member< Vector2 >& m) = 0;
	
	virtual bool operator >> (const Member< Vector4 >& m) = 0;
	
	virtual bool operator >> (const Member< Matrix33 >& m) = 0;
	
	virtual bool operator >> (const Member< Matrix44 >& m) = 0;

	virtual bool operator >> (const Member< Quaternion >& m) = 0;

	virtual bool operator >> (const Member< Serializable >& m) = 0;

	virtual bool operator >> (const Member< Serializable* >& m) = 0;

	virtual bool operator >> (const Member< void* >& m) = 0;

	virtual bool operator >> (const MemberArray& m) = 0;
	
	virtual bool operator >> (const MemberComplex& m) = 0;

	virtual bool operator >> (const MemberEnumBase& m);

	//@}

	template < typename T >
	T* readObject()
	{
		Ref< Serializable > object = readObject();
		return dynamic_type_cast< T* >(object);
	}

	template < typename T >
	T* getCurrentObject()
	{
		Ref< Serializable > object = getCurrentObject();
		return dynamic_type_cast< T* >(object);
	}

	template < typename T >
	T* getOuterObject()
	{
		Ref< Serializable > object = getOuterObject();
		return dynamic_type_cast< T* >(object);
	}

protected:
	bool serialize(Serializable* inner, int version, Serializable* outer);

private:
	std::list< std::pair< Ref< Serializable >, int > > m_constructing;
};

}

#endif	// traktor_Serializer_H
