/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/Split.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Ui/Custom/PropertyList/AnglesPropertyItem.h"
#include "Ui/Custom/PropertyList/ApplyReflector.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/CheckPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/ListPropertyItem.h"
#include "Ui/Custom/PropertyList/NullPropertyItem.h"
#include "Ui/Custom/PropertyList/NumericPropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/StaticPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/VectorPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ApplyReflector", ApplyReflector, Serializer)

ApplyReflector::ApplyReflector(AutoPropertyList* propertyList)
:	m_propertyList(propertyList)
{
	m_propertyList->getPropertyItems(m_propertyItems, PropertyList::GfDescendants);
	m_propertyItemIterator = m_propertyItems.begin();
}

Serializer::Direction ApplyReflector::getDirection() const
{
	return Serializer::SdRead;
}

void ApplyReflector::operator >> (const Member< bool >& m)
{
	CheckPropertyItem* propertyItem = checked_type_cast< CheckPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->isChecked();
}

void ApplyReflector::operator >> (const Member< int8_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int8_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< uint8_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint8_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< int16_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int16_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< uint16_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint16_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< int32_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int32_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< uint32_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint32_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< int64_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int64_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< uint64_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint64_t >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< float >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< float >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< double >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< double >(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< std::string >& m)
{
	TextPropertyItem* propertyItem = checked_type_cast< TextPropertyItem*, false >(*m_propertyItemIterator++);
	m = wstombs(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< std::wstring >& m)
{
	TextPropertyItem* propertyItem = checked_type_cast< TextPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
}

void ApplyReflector::operator >> (const Member< Guid >& m)
{
	BrowsePropertyItem* propertyItem = checked_type_cast< BrowsePropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
}

void ApplyReflector::operator >> (const Member< Path >& m)
{
	FilePropertyItem* propertyItem = checked_type_cast< FilePropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getPath();
}

void ApplyReflector::operator >> (const Member< Color4ub >& m)
{
	ColorPropertyItem* propertyItem = checked_type_cast< ColorPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
}

void ApplyReflector::operator >> (const Member< Color4f >& m)
{
	ColorPropertyItem* propertyItem = checked_type_cast< ColorPropertyItem*, false >(*m_propertyItemIterator++);
	const Color4ub& value = propertyItem->getValue();
	m = Color4f(value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f);
}

void ApplyReflector::operator >> (const Member< Scalar >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = Scalar(static_cast< float >(propertyItem->getValue()));
}

void ApplyReflector::operator >> (const Member< Vector2 >& m)
{
	VectorPropertyItem* propertyItem = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	const VectorPropertyItem::vector_t& value = propertyItem->getValue();
	m = Vector2(value[0], value[1]);
}

void ApplyReflector::operator >> (const Member< Vector4 >& m)
{
	PropertyItem* propertyItem = *m_propertyItemIterator++;
	if (VectorPropertyItem* vectorPropertyItem = dynamic_type_cast< VectorPropertyItem* >(propertyItem))
	{
		const VectorPropertyItem::vector_t& value = vectorPropertyItem->getValue();
		m = Vector4(value[0], value[1], value[2], value[3]);
	}
	else if (AnglesPropertyItem* anglesPropertyItem = dynamic_type_cast< AnglesPropertyItem* >(propertyItem))
	{
		m = anglesPropertyItem->getValue();
	}
}

void ApplyReflector::operator >> (const Member< Matrix33 >& m)
{
	PropertyItem* propertyItem = *m_propertyItemIterator++;

	VectorPropertyItem* propertyItemRow1 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	VectorPropertyItem* propertyItemRow2 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	VectorPropertyItem* propertyItemRow3 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);

	const VectorPropertyItem::vector_t& r1 = propertyItemRow1->getValue();
	const VectorPropertyItem::vector_t& r2 = propertyItemRow2->getValue();
	const VectorPropertyItem::vector_t& r3 = propertyItemRow3->getValue();

	m = Matrix33(
		r1[0], r1[1], r1[2],
		r2[0], r2[1], r2[2],
		r3[0], r3[1], r3[2]
	);
}

void ApplyReflector::operator >> (const Member< Matrix44 >& m)
{
	PropertyItem* propertyItem = *m_propertyItemIterator++;

	VectorPropertyItem* propertyItemRow1 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	VectorPropertyItem* propertyItemRow2 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	VectorPropertyItem* propertyItemRow3 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	VectorPropertyItem* propertyItemRow4 = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);

	const VectorPropertyItem::vector_t& r1 = propertyItemRow1->getValue();
	const VectorPropertyItem::vector_t& r2 = propertyItemRow2->getValue();
	const VectorPropertyItem::vector_t& r3 = propertyItemRow3->getValue();
	const VectorPropertyItem::vector_t& r4 = propertyItemRow4->getValue();

	m = Matrix44(
		r1[0], r1[1], r1[2], r1[3],
		r2[0], r2[1], r2[2], r2[3],
		r3[0], r3[1], r3[2], r3[3],
		r4[0], r4[1], r4[2], r4[3]
	);
}

void ApplyReflector::operator >> (const Member< Quaternion >& m)
{
	AnglesPropertyItem* propertyItem = checked_type_cast< AnglesPropertyItem*, false >(*m_propertyItemIterator++);
	m = Quaternion::fromEulerAngles(propertyItem->getValue());
}

void ApplyReflector::operator >> (const Member< ISerializable* >& m)
{
	ObjectPropertyItem* propertyItem = checked_type_cast< ObjectPropertyItem*, false >(*m_propertyItemIterator++);
	Ref< ISerializable > object = checked_type_cast< ISerializable* >(propertyItem->getObject());
	serialize(object);
	m = object;
}

void ApplyReflector::operator >> (const Member< void* >& m)
{
}

void ApplyReflector::operator >> (const MemberArray& m)
{
	ArrayPropertyItem* propertyItem = checked_type_cast< ArrayPropertyItem*, false >(*m_propertyItemIterator++);
	m.reserve(0, 0);
	while (m_propertyItemIterator != m_propertyItems.end() && (*m_propertyItemIterator)->getParentItem() == propertyItem)
	{
		if (!is_a< NullPropertyItem >(*m_propertyItemIterator))
			m.read(*this);
		else
		{
			m.insert();
			m_propertyItemIterator++;
		}
	}
}

void ApplyReflector::operator >> (const MemberComplex& m)
{
	if (m.getCompound())
		m_propertyItemIterator++;
	m.serialize(*this);
}

void ApplyReflector::operator >> (const MemberEnumBase& m)
{
	ListPropertyItem* propertyItem = checked_type_cast< ListPropertyItem*, false >(*m_propertyItemIterator++);
	m.set(propertyItem->getSelectedItem());
}

		}
	}
}
