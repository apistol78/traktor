#ifndef traktor_xml_XmlDeserializer_H
#define traktor_xml_XmlDeserializer_H

#include <map>
#include "Core/Serialization/Serializer.h"
#include "Xml/XmlPullParser.h"

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

/*! \brief XML based de-serializer.
 * \ingroup XML
 */
class T_DLLCLASS XmlDeserializer : public Serializer
{
	T_RTTI_CLASS;

public:
	XmlDeserializer(IStream* stream);

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
	XmlPullParser m_xpp;

	struct Entry
	{
		std::wstring name;
		int index;
		std::map< std::wstring, int > dups;
	};

	std::list< Entry > m_stack;
	std::map< std::wstring, Ref< ISerializable > > m_refs;
	std::wstring m_value;
	std::vector< float > m_values;

	std::wstring stackPath();

	bool enterElement(const std::wstring& name);

	bool leaveElement(const std::wstring& name);

	void rememberObject(ISerializable* object);
	
	bool nextElementValue(const std::wstring& name, std::wstring& value);
};
	
	}
}

#endif	// traktor_xml_XmlDeserializer_H
