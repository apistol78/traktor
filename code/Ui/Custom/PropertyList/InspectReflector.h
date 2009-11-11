#ifndef traktor_ui_custom_InspectReflector_H
#define traktor_ui_custom_InspectReflector_H

#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class AutoPropertyList;
class PropertyItem;

/*! \brief Auto property list inspect reflector.
 * \ingroup UIC
 */
class T_DLLCLASS InspectReflector : public Serializer
{
	T_RTTI_CLASS(InspectReflector)
	
public:
	InspectReflector(AutoPropertyList* propertyList, PropertyItem* parentPropertyItem = 0);
	
	virtual Serializer::Direction getDirection();

	virtual bool operator >> (const Member< bool >& m);
	
	virtual bool operator >> (const Member< int8_t >& m);
	
	virtual bool operator >> (const Member< uint8_t >& m);
	
	virtual bool operator >> (const Member< int16_t >& m);
	
	virtual bool operator >> (const Member< uint16_t >& m);
	
	virtual bool operator >> (const Member< int32_t >& m);
	
	virtual bool operator >> (const Member< uint32_t >& m);

	virtual bool operator >> (const Member< int64_t >& m);
	
	virtual bool operator >> (const Member< uint64_t >& m);
	
	virtual bool operator >> (const Member< float >& m);
	
	virtual bool operator >> (const Member< double >& m);
	
	virtual bool operator >> (const Member< std::string >& m);

	virtual bool operator >> (const Member< std::wstring >& m);

	virtual bool operator >> (const Member< Guid >& m);

	virtual bool operator >> (const Member< Path >& m);

	virtual bool operator >> (const Member< Color >& m);

	virtual bool operator >> (const Member< Scalar >& m);
	
	virtual bool operator >> (const Member< Vector2 >& m);
	
	virtual bool operator >> (const Member< Vector4 >& m);
	
	virtual bool operator >> (const Member< Matrix33 >& m);
	
	virtual bool operator >> (const Member< Matrix44 >& m);

	virtual bool operator >> (const Member< Quaternion >& m);
	
	virtual bool operator >> (const Member< Serializable >& m);

	virtual bool operator >> (const Member< Ref< Serializable > >& m);

	virtual bool operator >> (const Member< void* >& m);

	virtual bool operator >> (const MemberArray& m);
	
	virtual bool operator >> (const MemberComplex& m);

	virtual bool operator >> (const MemberEnumBase& m);
	
private:
	friend class AutoPropertyList;

	Ref< AutoPropertyList > m_propertyList;
	RefArray< PropertyItem > m_propertyItemStack;

	void addPropertyItem(PropertyItem* propertyItem);
};
		
		}
	}
}

#endif	// traktor_ui_custom_InspectReflector_H
