#pragma once

#include "Editor/IEditorTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_STORE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace store
	{

class T_DLLCLASS ImportAssetTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param) override final;
};

	}
}
