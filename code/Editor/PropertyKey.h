#ifndef traktor_editor_PropertyKey_H
#define traktor_editor_PropertyKey_H

#include "Core/Settings/IPropertyValue.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Key property.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyKey : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::pair< int, ui::VirtualKey > value_type_t;

	PropertyKey(const value_type_t& value = value_type_t(0, ui::VkNull));

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s);

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const;

	virtual Ref< IPropertyValue > clone() const;

private:
	value_type_t m_value;
};

	}
}

#endif	// traktor_editor_PropertyKey_H
