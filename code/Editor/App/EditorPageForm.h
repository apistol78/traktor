/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_EditorPageForm_H
#define traktor_editor_EditorPageForm_H

#include "Ui/Form.h"

namespace traktor
{
	namespace editor
	{
	
/*! \brief
 * \ingroup Editor
 */
class EditorPageForm : public ui::Form
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE;
};
	
	}
}

#endif	// traktor_editor_EditorPageForm_H
