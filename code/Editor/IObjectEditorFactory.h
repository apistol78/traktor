#pragma once

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

class ISerializable;

}

namespace traktor::ui
{

class Command;

}

namespace traktor::editor
{

class IEditor;
class IObjectEditor;

/*! Interface used by Editor to create object editors.
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

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const = 0;
};

}
