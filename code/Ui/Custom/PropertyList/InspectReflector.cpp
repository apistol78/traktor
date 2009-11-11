#include <sstream>
#include <limits>
#include "Ui/Custom/PropertyList/InspectReflector.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/StaticPropertyItem.h"
#include "Ui/Custom/PropertyList/NumericPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/VectorPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/CheckPropertyItem.h"
#include "Ui/Custom/PropertyList/ListPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"

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

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InspectReflector", InspectReflector, Serializer)

InspectReflector::InspectReflector(AutoPropertyList* propertyList, PropertyItem* parentPropertyItem)
:	m_propertyList(propertyList)
{
	if (parentPropertyItem)
		m_propertyItemStack.push_back(parentPropertyItem);
}

Serializer::Direction InspectReflector::getDirection()
{
	return Serializer::SdWrite;
}

bool InspectReflector::operator >> (const Member< bool >& m)
{
	addPropertyItem(gc_new< CheckPropertyItem >(stylizeMemberName(m.getName()), m));
	return true;
}

bool InspectReflector::operator >> (const Member< int8_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< int8_t >::min(),
		std::numeric_limits< int8_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< uint8_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< uint8_t >::min(),
		std::numeric_limits< uint8_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< int16_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< int16_t >::min(),
		std::numeric_limits< int16_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< uint16_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< uint16_t >::min(),
		std::numeric_limits< uint16_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< int32_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< int32_t >::min(),
		std::numeric_limits< int32_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< uint32_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< uint32_t >::min(),
		std::numeric_limits< uint32_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< int64_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< int64_t >::min(),
		std::numeric_limits< int64_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< uint64_t >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		std::numeric_limits< uint64_t >::min(),
		std::numeric_limits< uint64_t >::max(),
		false
	));
	return true;
}

bool InspectReflector::operator >> (const Member< float >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		m.getLimitMin(),
		m.getLimitMax(),
		true
	));
	return true;
}

bool InspectReflector::operator >> (const Member< double >& m)
{
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		m,
		-std::numeric_limits< double >::max(),
		std::numeric_limits< double >::max(),
		true
	));
	return true;
}

bool InspectReflector::operator >> (const Member< std::string >& m)
{
	addPropertyItem(gc_new< TextPropertyItem >(stylizeMemberName(m.getName()), mbstows(m), false));
	return true;
}

bool InspectReflector::operator >> (const Member< std::wstring >& m)
{
	addPropertyItem(gc_new< TextPropertyItem >(stylizeMemberName(m.getName()), m, m.isMultiLine()));
	return true;
}

bool InspectReflector::operator >> (const Member< Guid >& m)
{
	addPropertyItem(gc_new< BrowsePropertyItem >(stylizeMemberName(m.getName()), m.getType(), m));
	return true;
}

bool InspectReflector::operator >> (const Member< Path >& m)
{
	addPropertyItem(gc_new< FilePropertyItem >(stylizeMemberName(m.getName()), *m));
	return true;
}

bool InspectReflector::operator >> (const Member< Color >& m)
{
	addPropertyItem(gc_new< ColorPropertyItem >(stylizeMemberName(m.getName()), m));
	return true;
}

bool InspectReflector::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	addPropertyItem(gc_new< NumericPropertyItem >(
		stylizeMemberName(m.getName()),
		float(v),
		-std::numeric_limits< float >::max(),
		std::numeric_limits< float >::max(),
		true
	));
	return true;
}

bool InspectReflector::operator >> (const Member< Vector2 >& m)
{
	VectorPropertyItem::vector_t value = { m->x, m->y };
	addPropertyItem(gc_new< VectorPropertyItem >(stylizeMemberName(m.getName()), cref(value), 2));
	return true;
}

bool InspectReflector::operator >> (const Member< Vector4 >& m)
{
	VectorPropertyItem::vector_t value = { m->x(), m->y(), m->z(), m->w() };
	addPropertyItem(gc_new< VectorPropertyItem >(stylizeMemberName(m.getName()), cref(value), 4));
	return true;
}

bool InspectReflector::operator >> (const Member< Matrix33 >& m)
{
	return true;
}

bool InspectReflector::operator >> (const Member< Matrix44 >& m)
{
	return true;
}

bool InspectReflector::operator >> (const Member< Quaternion >& m)
{
	VectorPropertyItem::vector_t value = { m->x, m->y, m->z, m->w };
	addPropertyItem(gc_new< VectorPropertyItem >(stylizeMemberName(m.getName()), cref(value), 4));
	return true;
}

bool InspectReflector::operator >> (const Member< Serializable >& m)
{
	Ref< ObjectPropertyItem > propertyItem = gc_new< ObjectPropertyItem >(stylizeMemberName(m.getName()), m.getType(), &static_cast< Serializable& >(m));
	addPropertyItem(propertyItem);

	m_propertyItemStack.push_back(propertyItem);
	m->serialize(*this);
	m_propertyItemStack.pop_back();

	return true;
}

bool InspectReflector::operator >> (const Member< Ref< Serializable > >& m)
{
	Ref< Serializable > object = m;

	Ref< ObjectPropertyItem > propertyItem = gc_new< ObjectPropertyItem >(
		stylizeMemberName(m.getName()),
		m.getType(),
		object
	);
	addPropertyItem(propertyItem);

	if (object)
	{
		m_propertyItemStack.push_back(propertyItem);

		int version = object->getVersion();
		if (!serialize(object, version, 0))
			return false;

		m_propertyItemStack.pop_back();
	}
	
	return true;
}

bool InspectReflector::operator >> (const Member< void* >& m)
{
	return false;
}

bool InspectReflector::operator >> (const MemberArray& m)
{
	Ref< ArrayPropertyItem > propertyItem = gc_new< ArrayPropertyItem >(stylizeMemberName(m.getName()), m.getType());
	addPropertyItem(propertyItem);
	
	m_propertyItemStack.push_back(propertyItem);
	for (size_t i = 0; i < m.size(); ++i)
		m.serialize(*this, i);
	m_propertyItemStack.pop_back();

	return true;
}

bool InspectReflector::operator >> (const MemberComplex& m)
{
	if (m.getCompound())
	{
		Ref< PropertyItem > propertyItem = gc_new< PropertyItem >(stylizeMemberName(m.getName()));
		addPropertyItem(propertyItem);
		m_propertyItemStack.push_back(propertyItem);
	}

	if (!m.serialize(*this))
		return false;

	if (m.getCompound())
		m_propertyItemStack.pop_back();

	return true;
}

bool InspectReflector::operator >> (const MemberEnumBase& m)
{
	Ref< ListPropertyItem > propertyItem = gc_new< ListPropertyItem >(stylizeMemberName(m.getName()));
	addPropertyItem(propertyItem);

	for (const MemberEnumBase::Key* key = m.keys(); key && key->id; ++key)
	{
		int i = propertyItem->add(key->id);
		if (m.get() == key->id)
			propertyItem->select(i);
	}

	return true;
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
