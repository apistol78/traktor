/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IEditorPageFactory_H
#define traktor_editor_IEditorPageFactory_H

#include <list>
#include <set>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

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

class Command;

	}

	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPage;
class IEditorPageSite;

/*! \brief Interface for creating editor pages.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPageFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const = 0;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const = 0;

	virtual Ref< IEditorPage > createEditorPage(IEditor* editor, IEditorPageSite* site, IDocument* document) const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;
};

	}
}

#endif	// traktor_editor_IEditorPageFactory_H
