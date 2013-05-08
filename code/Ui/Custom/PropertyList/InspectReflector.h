#ifndef traktor_ui_custom_InspectReflector_H
#define traktor_ui_custom_InspectReflector_H

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;
	
public:
	InspectReflector(AutoPropertyList* propertyList, PropertyItem* parentPropertyItem = 0);
	
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
	friend class AutoPropertyList;

	Ref< AutoPropertyList > m_propertyList;
	RefArray< PropertyItem > m_propertyItemStack;

	void addPropertyItem(PropertyItem* propertyItem);
};
		
		}
	}
}

#endif	// traktor_ui_custom_InspectReflector_H
