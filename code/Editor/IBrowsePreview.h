/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IBrowsePreview_H
#define traktor_editor_IBrowsePreview_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace ui
	{

class Bitmap;

	}

	namespace editor
	{

class IEditor;

/*! \brief Browse instance preview generator interface.
 * \ingroup Editor
 */
class T_DLLCLASS IBrowsePreview : public Object
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const = 0;

	virtual Ref< ui::Bitmap > generate(const IEditor* editor, db::Instance* instance) const = 0;
};

	}
}

#endif	// traktor_editor_IBrowsePreview_H
