#ifndef traktor_ui_custom_AutoPropertyList_H
#define traktor_ui_custom_AutoPropertyList_H

#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Core/Serialization/ISerializable.h"

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

/*! \brief Auto property list.
 * \ingroup UIC
 */
class T_DLLCLASS AutoPropertyList : public PropertyList
{
	T_RTTI_CLASS;
	
public:
	bool bind(ISerializable* object);

	bool refresh();

	bool refresh(PropertyItem* parent, ISerializable* object);

	bool apply();

	/*! \brief Add object to property list.
	 *
	 * Use this method when adding an object to an array item.
	 */
	bool addObject(PropertyItem* parent, ISerializable* object);

	virtual bool paste();
	
private:
	Ref< ISerializable > m_object;
};
		
		}
	}
}

#endif	// traktor_ui_custom_AutoPropertyList_H
