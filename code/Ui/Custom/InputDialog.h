/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_InputDialog_H
#define traktor_ui_custom_InputDialog_H

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

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

class EditValidator;

		namespace custom
		{

/*! \brief User input dialog.
 * \ingroup UIC
 */
class T_DLLCLASS InputDialog : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	struct Field
	{
		std::wstring title;
		std::wstring value;
		Ref< EditValidator > validator;
		const wchar_t** values;
		bool browseFile;

		Field()
		:	values(0)
		,	browseFile(false)
		{
		}

		Field(
			const std::wstring& title_,
			const std::wstring& value_ = L"",
			EditValidator* validator_ = 0,
			const wchar_t** values_ = 0,
			bool browseFile_ = false
		)
		:	title(title_)
		,	value(value_)
		,	validator(validator_)
		,	values(values_)
		,	browseFile(browseFile_)
		{
		}
	};

	InputDialog();

	bool create(
		Widget* parent,
		const std::wstring& title,
		const std::wstring& message,
		Field* outFields,
		uint32_t outFieldsCount
	);

	virtual int showModal() T_OVERRIDE;

private:
	Field* m_outFields;
	RefArray< Widget > m_editFields;
};

		}
	}
}

#endif	// traktor_ui_custom_InputDialog_H
