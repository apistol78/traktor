/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IEditorPageSite_H
#define traktor_editor_IEditorPageSite_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace editor
	{

/*! \brief
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPageSite : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Attach object to property list. */
	virtual void setPropertyObject(Object* properties) = 0;

	/*! \brief Create additional docking panel. */
	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south) = 0;

	/*! \brief Destroy additional docking panel. */
	virtual void destroyAdditionalPanel(ui::Widget* widget) = 0;

	/*! \brief Show additional docking panel. */
	virtual void showAdditionalPanel(ui::Widget* widget) = 0;

	/*! \brief Hide additional docking panel. */
	virtual void hideAdditionalPanel(ui::Widget* widget) = 0;
};

	}
}

#endif	// traktor_editor_IEditorPageSite_H
