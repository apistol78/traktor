#ifndef traktor_xml_XmlSerializer_H
#define traktor_xml_XmlSerializer_H

#include <map>
#include "Core/Ref.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace xml
	{

/*! \brief XML based serializer.
 * \ingroup XML
 */
class T_DLLCLASS XmlSerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	XmlSerializer(IStream* stream);
	
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
	FileOutputStream m_xml;
	std::wstring m_indent;

	struct Entry
	{
		std::wstring name;
		int index;
		std::map< std::wstring, int > dups;
	};

	std::list< Entry > m_stack;
	std::map< ISerializable*, std::wstring > m_refs;

	std::wstring stackPath();

	void enterElement(const std::wstring& name);

	void leaveElement();

	void rememberObject(ISerializable* object);

	void incrementIndent();

	void decrementIndent();
};
	
	}
}

#endif	// traktor_xml_XmlSerializer_H
