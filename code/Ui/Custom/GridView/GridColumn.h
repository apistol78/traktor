/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_GridColumn_H
#define traktor_ui_GridColumn_H

#include "Core/Object.h"
#include "Ui/Associative.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Grid column.
 * \ingroup UIC
 */
class T_DLLCLASS GridColumn
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	GridColumn(const std::wstring& title, uint32_t width);

	GridColumn(const std::wstring& title, uint32_t width, bool editable);

	void setTitle(const std::wstring& title);
	
	const std::wstring& getTitle() const;

	void setWidth(uint32_t width);

	uint32_t getWidth() const;

	void setEditable(bool editable);

	bool isEditable() const;

private:
	std::wstring m_title;
	uint32_t m_width;
	bool m_editable;
};

		}
	}
}

#endif	// traktor_ui_GridColumn_H
