#include <limits>
#include <sstream>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeHex.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePoint.h"
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
	addPropertyItem(new CheckPropertyItem(stylizeMemberName(m.getName()), m));
}

void InspectReflector::operator >> (const Member< int8_t >& m)
{
	float min = std::numeric_limits< int8_t >::min();
	float max = std::numeric_limits< int8_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false
	));
}

void InspectReflector::operator >> (const Member< uint8_t >& m)
{
	float min = std::numeric_limits< uint8_t >::min();
	float max = std::numeric_limits< uint8_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false,
		hex != 0
	));
}

void InspectReflector::operator >> (const Member< int16_t >& m)
{
	float min = std::numeric_limits< int16_t >::min();
	float max = std::numeric_limits< int16_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false
	));
}

void InspectReflector::operator >> (const Member< uint16_t >& m)
{
	float min = std::numeric_limits< uint16_t >::min();
	float max = std::numeric_limits< uint16_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false,
		hex != 0
	));
}

void InspectReflector::operator >> (const Member< int32_t >& m)
{
	float min = std::numeric_limits< int32_t >::min();
	float max = std::numeric_limits< int32_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false
	));
}

void InspectReflector::operator >> (const Member< uint32_t >& m)
{
	double min = std::numeric_limits< uint32_t >::min();
	double max = std::numeric_limits< uint32_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		false,
		hex != 0
	));
}

void InspectReflector::operator >> (const Member< int64_t >& m)
{
	double min = std::numeric_limits< int64_t >::min();
	double max = std::numeric_limits< int64_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		(double)m,
		min,
		max,
		false
	));
}

void InspectReflector::operator >> (const Member< uint64_t >& m)
{
	double min = std::numeric_limits< uint64_t >::min();
	double max = std::numeric_limits< uint64_t >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	const AttributeHex* hex = findAttribute< AttributeHex >(m);

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		(double)m,
		min,
		max,
		false,
		hex != 0
	));
}

void InspectReflector::operator >> (const Member< float >& m)
{
	float min = -std::numeric_limits< float >::max();
	float max = std::numeric_limits< float >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		true
	));
}

void InspectReflector::operator >> (const Member< double >& m)
{
	double min = -std::numeric_limits< double >::max();
	double max = std::numeric_limits< double >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		min,
		max,
		true
	));
}

void InspectReflector::operator >> (const Member< std::string >& m)
{
	addPropertyItem(new TextPropertyItem(stylizeMemberName(m.getName()), mbstows(m), false));
}

void InspectReflector::operator >> (const Member< std::wstring >& m)
{
	const AttributeMultiLine* multiLine = findAttribute< AttributeMultiLine >(m);
	addPropertyItem(new TextPropertyItem(
		stylizeMemberName(m.getName()),
		m,
		multiLine != 0
	));
}

void InspectReflector::operator >> (const Member< Guid >& m)
{
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	addPropertyItem(new BrowsePropertyItem(stylizeMemberName(
		m.getName()),
		memberType ? &(memberType->getMemberType()) : 0,
		m
	));
}

void InspectReflector::operator >> (const Member< Path >& m)
{
	addPropertyItem(new FilePropertyItem(stylizeMemberName(m.getName()), *m));
}

void InspectReflector::operator >> (const Member< Color4ub >& m)
{
	addPropertyItem(new ColorPropertyItem(stylizeMemberName(m.getName()), m));
}

void InspectReflector::operator >> (const Member< Color4f >& m)
{
	Color4ub value(
		uint8_t(m->getRed() * 255),
		uint8_t(m->getGreen() * 255),
		uint8_t(m->getBlue() * 255),
		uint8_t(m->getAlpha() * 255)
	);
	addPropertyItem(new ColorPropertyItem(stylizeMemberName(m.getName()), value));
}

void InspectReflector::operator >> (const Member< Scalar >& m)
{
	float min = std::numeric_limits< float >::min();
	float max = std::numeric_limits< float >::max();

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		min = range->getMin();
		max = range->getMax();
	}

	Scalar& v = m;
	addPropertyItem(new NumericPropertyItem(
		stylizeMemberName(m.getName()),
		float(v),
		min,
		max,
		true
	));
}

void InspectReflector::operator >> (const Member< Vector2 >& m)
{
	VectorPropertyItem::vector_t value = { m->x, m->y };
	addPropertyItem(new VectorPropertyItem(stylizeMemberName(m.getName()), value, 2));
}

void InspectReflector::operator >> (const Member< Vector4 >& m)
{
	const AttributeDirection* direction = findAttribute< AttributeDirection >(m);
	if (direction)
	{
		VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), 0.0f };
		addPropertyItem(new VectorPropertyItem(stylizeMemberName(m.getName()), value, 3));
		return;
	}

	const AttributePoint* point = findAttribute< AttributePoint >(m);
	if (point)
	{
		VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), 1.0f };
		addPropertyItem(new VectorPropertyItem(stylizeMemberName(m.getName()), value, 3));
		return;
	}

	const AttributeAngles* angles = findAttribute< AttributeAngles >(m);
	if (angles)
	{
		addPropertyItem(new AnglesPropertyItem(stylizeMemberName(m.getName()), *m));
		return;
	}

	VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), m->w() };
	addPropertyItem(new VectorPropertyItem(stylizeMemberName(m.getName()), value, 4));
}

void InspectReflector::operator >> (const Member< Matrix33 >& m)
{
}

void InspectReflector::operator >> (const Member< Matrix44 >& m)
{
}

void InspectReflector::operator >> (const Member< Quaternion >& m)
{
	addPropertyItem(new AnglesPropertyItem(stylizeMemberName(m.getName()), m->toEulerAngles()));
}

void InspectReflector::operator >> (const Member< ISerializable* >& m)
{
	const AttributeType* memberType = findAttribute< AttributeType >(m);

	Ref< ISerializable > object = *m;

	Ref< ObjectPropertyItem > propertyItem = new ObjectPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : 0,
		object
	);
	addPropertyItem(propertyItem);

	if (object)
	{
		m_propertyItemStack.push_back(propertyItem);

		int32_t version = type_of(object).getVersion();
		serialize(object, version);

		m_propertyItemStack.pop_back();
	}
}

void InspectReflector::operator >> (const Member< void* >& m)
{
}

void InspectReflector::operator >> (const MemberArray& m)
{
	const AttributeType* memberType = findAttribute< AttributeType >(m);
	const AttributeReadOnly* memberReadOnly = findAttribute< AttributeReadOnly >(m);

	Ref< ArrayPropertyItem > propertyItem = new ArrayPropertyItem(
		stylizeMemberName(m.getName()),
		memberType ? &(memberType->getMemberType()) : 0,
		memberReadOnly ? true : false
	);
	addPropertyItem(propertyItem);
	
	m_propertyItemStack.push_back(propertyItem);
	for (size_t i = 0; i < m.size(); ++i)
		m.write(*this);
	m_propertyItemStack.pop_back();
}

void InspectReflector::operator >> (const MemberComplex& m)
{
	if (m.getCompound())
	{
		Ref< PropertyItem > propertyItem = new PropertyItem(stylizeMemberName(m.getName()));
		addPropertyItem(propertyItem);
		m_propertyItemStack.push_back(propertyItem);
	}

	m.serialize(*this);

	if (m.getCompound())
		m_propertyItemStack.pop_back();
}

void InspectReflector::operator >> (const MemberEnumBase& m)
{
	Ref< ListPropertyItem > propertyItem = new ListPropertyItem(stylizeMemberName(m.getName()));
	addPropertyItem(propertyItem);

	for (const MemberEnumBase::Key* key = m.keys(); key && key->id; ++key)
	{
		int i = propertyItem->add(key->id);
		if (m.get() == key->id)
			propertyItem->select(i);
	}
}

void InspectReflector::addPropertyItem(PropertyItem* propertyItem)
{
	if (m_propertyItemStack.empty())
		m_propertyList->addPropertyItem(propertyItem);
	else
		m_propertyList->addPropertyItem(m_propertyItemStack.back(), propertyItem);
}

		}
	}
}
