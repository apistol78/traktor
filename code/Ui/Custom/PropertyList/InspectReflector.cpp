/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include <sstream>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeDecibel.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeHex.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Ui/Custom/PropertyList/AnglesPropertyItem.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/CheckPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/InspectReflector.h"
#include "Ui/Custom/PropertyList/ListPropertyItem.h"
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
			namespace
			{

/*! \brief Stylize member name.
 *
 * Transform from internal naming convention to
 * more human acceptable form.
 *
 * Example:
 * thisIsACommonName -> This is a common name
 *
 * \param memberName Name of member in internal naming convention.
 * \return Human acceptable form.
 */
std::wstring stylizeMemberName(const std::wstring& memberName)
{
	T_ASSERT (!memberName.empty());
	StringOutputStream ss;
	std::wstring::const_iterator i = memberName.begin();
	ss.put(toupper(*i++));
	for (; i != memberName.end(); ++i)
	{
		if (isupper(*i))
		{
			ss.put(L' ');
			ss.put(tolower(*i));
		}
		else
			ss.put(*i);
	}
	return ss.str();
}

template < typename AttributeType, typename MemberType >
const AttributeType* findAttribute(const MemberType& m)
{
	const Attribute* attributes = m.getAttributes();
	return attributes ? attributes->find< AttributeType >() : 0;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InspectReflector", InspectReflector, Serializer)

InspectReflector::InspectReflector(AutoPropertyList* propertyList, PropertyItem* parentPropertyItem)
:	m_propertyList(propertyList)
{
	if (parentPropertyItem)
		m_propertyItemStack.push_back(parentPropertyItem);
}

Serializer::Direction InspectReflector::getDirection() const
{
	return Serializer::SdWrite;
}

void InspectReflector::operator >> (const Member< bool >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	addPropertyItem(
		new CheckPropertyItem(stylizeMemberName(m.getName()), m),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int8_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< int8_t >::min();
	float max = std::numeric_limits< int8_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			false,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint8_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< uint8_t >::min();
	float max = std::numeric_limits< uint8_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			hex != 0,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int16_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< int16_t >::min();
	float max = std::numeric_limits< int16_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			false,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint16_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< uint16_t >::min();
	float max = std::numeric_limits< uint16_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			hex != 0,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int32_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< int32_t >::min();
	float max = std::numeric_limits< int32_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			false,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint32_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	double min = std::numeric_limits< uint32_t >::min();
	double max = std::numeric_limits< uint32_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			false,
			hex != 0,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int64_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	double min = std::numeric_limits< int64_t >::min();
	double max = std::numeric_limits< int64_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			(double)m,
			min,
			max,
			false,
			false,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint64_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	double min = std::numeric_limits< uint64_t >::min();
	double max = std::numeric_limits< uint64_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			(double)m,
			min,
			max,
			false,
			hex != 0,
			false
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< float >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = -std::numeric_limits< float >::max();
	float max = std::numeric_limits< float >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			true,
			false,
			bool(findAttribute< AttributeDecibel >(m) != 0)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< double >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	double min = -std::numeric_limits< double >::max();
	double max = std::numeric_limits< double >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			min,
			max,
			true,
			false,
			bool(findAttribute< AttributeDecibel >(m) != 0)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< std::string >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	addPropertyItem(
		new TextPropertyItem(stylizeMemberName(m.getName()), mbstows(m), false),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< std::wstring >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const AttributeMultiLine* multiLine = findAttribute< AttributeMultiLine >(m);
	addPropertyItem(
		new TextPropertyItem(
			stylizeMemberName(m.getName()),
			m,
			multiLine != 0
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Guid >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	addPropertyItem(
		new BrowsePropertyItem(stylizeMemberName(
			m.getName()),
			memberType ? &(memberType->getMemberType()) : 0,
			m
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Path >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	addPropertyItem(
		new FilePropertyItem(stylizeMemberName(m.getName()), *m),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Color4ub >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	addPropertyItem(
		new ColorPropertyItem(stylizeMemberName(m.getName()), m),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Color4f >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	Color4ub value(
		uint8_t(m->getRed() * 255),
		uint8_t(m->getGreen() * 255),
		uint8_t(m->getBlue() * 255),
		uint8_t(m->getAlpha() * 255)
	);
	addPropertyItem(
		new ColorPropertyItem(stylizeMemberName(m.getName()), value),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Scalar >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	float min = std::numeric_limits< float >::min();
	float max = std::numeric_limits< float >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	Scalar& v = m;
	addPropertyItem(
		new NumericPropertyItem(
			stylizeMemberName(m.getName()),
			float(v),
			min,
			max,
			true,
			false,
			bool(findAttribute< AttributeDecibel >(m) != 0)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Vector2 >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const VectorPropertyItem::vector_t value = { m->x, m->y };
	addPropertyItem(
		new VectorPropertyItem(stylizeMemberName(m.getName()), value, 2),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Vector4 >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	const AttributeDirection* direction = findAttribute< AttributeDirection >(m);
	if (direction)
	{
		VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), 0.0f };
		addPropertyItem(
			new VectorPropertyItem(stylizeMemberName(m.getName()), value, 3),
			memberPrivate
		);
		return;
	}

	const AttributePoint* point = findAttribute< AttributePoint >(m);
	if (point)
	{
		VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), 1.0f };
		addPropertyItem(
			new VectorPropertyItem(stylizeMemberName(m.getName()), value, 3),
			memberPrivate
		);
		return;
	}

	const AttributeAngles* angles = findAttribute< AttributeAngles >(m);
	if (angles)
	{
		addPropertyItem(
			new AnglesPropertyItem(stylizeMemberName(m.getName()), *m),
			memberPrivate
		);
		return;
	}

	VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), m->w() };
	addPropertyItem(
		new VectorPropertyItem(stylizeMemberName(m.getName()), value, 4),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Matrix33 >& m)
{
	Ref< PropertyItem > propertyItem = new PropertyItem(
		stylizeMemberName(m.getName())
	);
	addPropertyItem(propertyItem, false);

	VectorPropertyItem::vector_t row1 = { m->e[0][0], m->e[0][1], m->e[0][2] };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[0]", row1, 3));

	VectorPropertyItem::vector_t row2 = { m->e[1][0], m->e[1][1], m->e[1][2] };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[1]", row2, 3));

	VectorPropertyItem::vector_t row3 = { m->e[2][0], m->e[2][1], m->e[2][2] };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[2]", row3, 3));
}

void InspectReflector::operator >> (const Member< Matrix44 >& m)
{
	Ref< PropertyItem > propertyItem = new PropertyItem(
		stylizeMemberName(m.getName())
	);
	addPropertyItem(propertyItem, false);

	VectorPropertyItem::vector_t row1 = { m->get(0, 0), m->get(0, 1), m->get(0, 2), m->get(0, 3) };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[0]", row1, 4));

	VectorPropertyItem::vector_t row2 = { m->get(1, 0), m->get(1, 1), m->get(1, 2), m->get(1, 3) };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[1]", row2, 4));

	VectorPropertyItem::vector_t row3 = { m->get(2, 0), m->get(2, 1), m->get(2, 2), m->get(2, 3) };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[2]", row3, 4));

	VectorPropertyItem::vector_t row4 = { m->get(3, 0), m->get(3, 1), m->get(3, 2), m->get(3, 3) };
	m_propertyList->addPropertyItem(propertyItem, new VectorPropertyItem(L"[3]", row4, 4));
}

void InspectReflector::operator >> (const Member< Quaternion >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	addPropertyItem(
		new AnglesPropertyItem(stylizeMemberName(m.getName()), m->toEulerAngles()),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< ISerializable* >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const AttributeType* memberType = findAttribute< AttributeType >(m);

	Ref< ISerializable > object = *m;

	Ref< ObjectPropertyItem > propertyItem = new ObjectPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : 0,
		object
	);
	addPropertyItem(propertyItem, memberPrivate);

	if (object)
	{
		m_propertyItemStack.push_back(propertyItem);
		serialize(object);
		m_propertyItemStack.pop_back();
	}
}

void InspectReflector::operator >> (const Member< void* >& m)
{
}

void InspectReflector::operator >> (const MemberArray& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	const AttributeReadOnly* memberReadOnly = findAttribute< AttributeReadOnly >(m);

	Ref< ArrayPropertyItem > propertyItem = new ArrayPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : 0,
		memberReadOnly ? true : false
	);
	addPropertyItem(propertyItem, memberPrivate);
	
	m_propertyItemStack.push_back(propertyItem);
	for (size_t i = 0; i < m.size(); ++i)
		m.write(*this);
	m_propertyItemStack.pop_back();
}

void InspectReflector::operator >> (const MemberComplex& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);

	if (m.getCompound())
	{
		Ref< PropertyItem > propertyItem = new PropertyItem(stylizeMemberName(m.getName()));
		addPropertyItem(propertyItem, memberPrivate);
		m_propertyItemStack.push_back(propertyItem);
	}

	m.serialize(*this);

	if (m.getCompound())
		m_propertyItemStack.pop_back();
}

void InspectReflector::operator >> (const MemberEnumBase& m)
{
	Ref< ListPropertyItem > propertyItem = new ListPropertyItem(stylizeMemberName(m.getName()));
	addPropertyItem(propertyItem, false);

	for (const MemberEnumBase::Key* key = m.keys(); key && key->id; ++key)
	{
		int32_t i = propertyItem->add(key->id);
		if (m.get() == key->id)
			propertyItem->select(i);
	}
}

void InspectReflector::addPropertyItem(PropertyItem* propertyItem, bool propertyPrivate)
{
	// Check if child of a private property; privacy is propagated to children.
	if (!propertyPrivate)
	{
		for (RefArray< PropertyItem >::const_iterator i = m_propertyItemStack.begin(); i != m_propertyItemStack.end(); ++i)
		{
			if (!(*i)->isVisible())
				propertyPrivate = true;
		}
	}
	if (propertyPrivate)
		propertyItem->setVisible(false);

	// Add property item to parent or as root.
	if (m_propertyItemStack.empty())
		m_propertyList->addPropertyItem(propertyItem);
	else
		m_propertyList->addPropertyItem(m_propertyItemStack.back(), propertyItem);
}

		}
	}
}
