/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Associative_H
#define traktor_ui_Associative_H

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! \brief Associate container.
 * \ingroup UI
 *
 * Each UI widget is derived from this container to make it
 * possible to attach objects by names to widgets or widget items.
 */
class T_DLLCLASS Associative
{
public:
	virtual ~Associative();

	/*! \brief Remove all objects. */
	void removeAllData();

	/*! \brief Set named object. */
	void setData(const std::wstring& key, Object* data);

	/*! \brief Get named object. */
	Ref< Object > getData(const std::wstring& key) const;

	/*! \brief Get named object, ensure proper type. */
	template < typename DataType >
	Ref< DataType > getData(const std::wstring& key) const
	{
		Ref< Object > data = getData(key);
		return dynamic_type_cast< DataType* >(data);
	}

private:
	std::map< std::wstring, Ref< Object > > m_data;
};

	}
}

#endif	// traktor_ui_Associative_H
