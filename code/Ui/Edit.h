/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Edit_H
#define traktor_ui_Edit_H

#include "Ui/Widget.h"

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

class EditValidator;

/*! \brief Text edit.
 * \ingroup UI
 */
class T_DLLCLASS Edit : public Widget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsReadOnly = WsUser
	};

	bool create(Widget* parent, const std::wstring& text = L"", int style = WsClientBorder, const EditValidator* validator = 0);
	
	bool setValidator(const EditValidator* validator);

	const EditValidator* getValidator() const;

	void setSelection(int from, int to);

	void selectAll();

	void setBorderColor(const Color4ub& borderColor);

private:
	Ref< const EditValidator > m_validator;

	void eventKey(KeyEvent* event);
};

	}
}

#endif	// traktor_ui_Edit_H
