/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include <sstream>
#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Half.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Misc/Split.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeHex.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Ui/PropertyList/AnglesPropertyItem.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/AutoPropertyList.h"
#include "Ui/PropertyList/BrowsePropertyItem.h"
#include "Ui/PropertyList/CheckPropertyItem.h"
#include "Ui/PropertyList/ColorPropertyItem.h"
#include "Ui/PropertyList/FilePropertyItem.h"
#include "Ui/PropertyList/InspectReflector.h"
#include "Ui/PropertyList/ListPropertyItem.h"
#include "Ui/PropertyList/NumericPropertyItem.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/StaticPropertyItem.h"
#include "Ui/PropertyList/TextPropertyItem.h"
#include "Ui/PropertyList/VectorPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

/*! Stylize member name.
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
	T_ASSERT(!memberName.empty());
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
	return attributes ? attributes->find< AttributeType >() : nullptr;
}

template < typename MemberType >
NumericPropertyItem::Representation findRepresentation(const MemberType& m)
{
	const AttributeUnit* unit = findAttribute< AttributeUnit >(m);
	if (unit)
	{
		if (unit->getUnit() == UnitType::Metres)
			return unit->getPerSecond() ? NumericPropertyItem::RpMetresPerSecond : NumericPropertyItem::RpMetres;
		else if (unit->getUnit() == UnitType::Radians)
			return unit->getPerSecond() ? NumericPropertyItem::RpAnglesPerSecond : NumericPropertyItem::RpAngle;
		else if (unit->getUnit() == UnitType::Decibel)
			return NumericPropertyItem::RpDecibel;
		else if (unit->getUnit() == UnitType::Kilograms)
			return NumericPropertyItem::RpKilograms;
		else if (unit->getUnit() == UnitType::Percent)
			return NumericPropertyItem::RpPercent;
		else if (unit->getUnit() == UnitType::Hertz)
			return NumericPropertyItem::RpHerz;
		else if (unit->getUnit() == UnitType::EV)
			return NumericPropertyItem::RpEV;
		else if (unit->getUnit() == UnitType::Newton)
			return NumericPropertyItem::RpNewton;
		else if (unit->getUnit() == UnitType::NewtonSecond)
			return NumericPropertyItem::RpNewtonSecond;
	}
	return NumericPropertyItem::RpNormal;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.InspectReflector", InspectReflector, Serializer)

InspectReflector::InspectReflector(AutoPropertyList* propertyList, PropertyItem* parentPropertyItem)
:	m_propertyList(propertyList)
{
	if (parentPropertyItem)
		m_propertyItemStack.push_back(parentPropertyItem);
}

Serializer::Direction InspectReflector::getDirection() const
{
	return Serializer::Direction::Write;
}

void InspectReflector::operator >> (const Member< bool >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	addPropertyItem(
		new CheckPropertyItem(stylizeMemberName(m.getName()), m),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int8_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	float min = -std::numeric_limits< int8_t >::max();
	float max =  std::numeric_limits< int8_t >::max();

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
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint8_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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
			hex != nullptr,
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int16_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	float min = -std::numeric_limits< int16_t >::max();
	float max =  std::numeric_limits< int16_t >::max();

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
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint16_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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
			hex != nullptr,
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int32_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	float min = -std::numeric_limits< float >::max();
	float max =  std::numeric_limits< float >::max();

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
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint32_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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
			hex != nullptr,
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< int64_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	double min = -std::numeric_limits< int64_t >::max();
	double max =  std::numeric_limits< int64_t >::max();

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
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< uint64_t >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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
			hex != nullptr,
			NumericPropertyItem::RpNormal
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< float >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	float min = -std::numeric_limits< float >::max();
	float max =  std::numeric_limits< float >::max();

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
			findRepresentation(m)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< double >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

	double min = -std::numeric_limits< double >::max();
	double max =  std::numeric_limits< double >::max();

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
			findRepresentation(m)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< std::string >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
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
			multiLine != nullptr
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Guid >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	addPropertyItem(
		new BrowsePropertyItem(stylizeMemberName(
			m.getName()),
			memberType ? &(memberType->getMemberType()) : nullptr,
			m
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Path >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	addPropertyItem(
		new FilePropertyItem(stylizeMemberName(m.getName()), *m),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Color4ub >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	Color4f value(
		m->r / 255.0f,
		m->g / 255.0f,
		m->b / 255.0f,
		m->a / 255.0f
	);
	addPropertyItem(
		new ColorPropertyItem(stylizeMemberName(m.getName()), value, false),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Color4f >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != 0);
	const bool memberHdr = bool(findAttribute< AttributeHdr >(m) != 0);
	addPropertyItem(
		new ColorPropertyItem(stylizeMemberName(m.getName()), m, memberHdr),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Scalar >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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
			findRepresentation(m)
		),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Vector2 >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	const VectorPropertyItem::vector_t value = { m->x, m->y };
	addPropertyItem(
		new VectorPropertyItem(stylizeMemberName(m.getName()), value, 2),
		memberPrivate
	);
}

void InspectReflector::operator >> (const Member< Vector4 >& m)
{
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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

	const AttributeUnit* unit = findAttribute< AttributeUnit >(m);
	if (unit != nullptr && unit->getUnit() == UnitType::Degrees)
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
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	const AttributeType* memberType = findAttribute< AttributeType >(m);

	Ref< ISerializable > object = *m;

	Ref< ObjectPropertyItem > propertyItem = new ObjectPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : nullptr,
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
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	const AttributeReadOnly* memberReadOnly = findAttribute< AttributeReadOnly >(m);

	Ref< ArrayPropertyItem > propertyItem = new ArrayPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : nullptr,
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
	const bool memberPrivate = bool(findAttribute< AttributePrivate >(m) != nullptr);

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

	m.enumerate([&](const wchar_t* id) {
		int32_t i = propertyItem->add(id);
		if (m.get() == id)
			propertyItem->select(i);		
	});
}

void InspectReflector::addPropertyItem(PropertyItem* propertyItem, bool propertyPrivate)
{
	// Check if child of a private property; privacy is propagated to children.
	if (!propertyPrivate)
	{
		for (auto item : m_propertyItemStack)
		{
			if (!item->isVisible())
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
