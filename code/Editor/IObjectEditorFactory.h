/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IObjectEditorFactory_H
#define traktor_editor_IObjectEditorFactory_H

#include <list>
#include <set>
#include "Core/Object.h"
#include "Core/Guid.h"

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

class IEditor;
class IObjectEditor;

/*! \brief Interface used by Editor to create object editors.
 * \ingroup Editor
 */
class T_DLLCLASS IObjectEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const = 0;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const = 0;

	virtual Ref< IObjectEditor > createObjectEditor(IEditor* editor) const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;
};

	}
}

#endif	// traktor_editor_IObjectEditorFactory_H
