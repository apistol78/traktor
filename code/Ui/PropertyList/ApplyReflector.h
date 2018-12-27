/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ApplyReflector_H
#define traktor_ui_ApplyReflector_H

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class AutoPropertyList;
class PropertyItem;

/*! \brief Auto property list apply reflector.
 * \ingroup UI
 */
class T_DLLCLASS ApplyReflector : public Serializer
{
	T_RTTI_CLASS;
	
public:
	ApplyReflector(AutoPropertyList* propertyList);
	
	virtual Direction getDirection() const override final;

	virtual void operator >> (const Member< bool >& m) override final;
	
	virtual void operator >> (const Member< int8_t >& m) override final;
	
	virtual void operator >> (const Member< uint8_t >& m) override final;
	
	virtual void operator >> (const Member< int16_t >& m) override final;
	
	virtual void operator >> (const Member< uint16_t >& m) override final;
	
	virtual void operator >> (const Member< int32_t >& m) override final;
	
	virtual void operator >> (const Member< uint32_t >& m) override final;

	virtual void operator >> (const Member< int64_t >& m) override final;
	
	virtual void operator >> (const Member< uint64_t >& m) override final;
	
	virtual void operator >> (const Member< float >& m) override final;
	
	virtual void operator >> (const Member< double >& m) override final;
	
	virtual void operator >> (const Member< std::string >& m) override final;

	virtual void operator >> (const Member< std::wstring >& m) override final;

	virtual void operator >> (const Member< Guid >& m) override final;

	virtual void operator >> (const Member< Path >& m) override final;

	virtual void operator >> (const Member< Color4ub >& m) override final;

	virtual void operator >> (const Member< Color4f >& m) override final;

	virtual void operator >> (const Member< Scalar >& m) override final;
	
	virtual void operator >> (const Member< Vector2 >& m) override final;
	
	virtual void operator >> (const Member< Vector4 >& m) override final;
	
	virtual void operator >> (const Member< Matrix33 >& m) override final;
	
	virtual void operator >> (const Member< Matrix44 >& m) override final;

	virtual void operator >> (const Member< Quaternion >& m) override final;

	virtual void operator >> (const Member< ISerializable* >& m) override final;

	virtual void operator >> (const Member< void* >& m) override final;

	virtual void operator >> (const MemberArray& m) override final;
	
	virtual void operator >> (const MemberComplex& m) override final;

	virtual void operator >> (const MemberEnumBase& m) override final;

private:
	friend class AutoPropertyList;

	AutoPropertyList* m_propertyList;
	RefArray< PropertyItem > m_propertyItems;
	RefArray< PropertyItem >::iterator m_propertyItemIterator;
};
		
	}
}

#endif	// traktor_ui_ApplyReflector_H
