#pragma once

#include "Editor/IWizardTool.h"

namespace traktor
{
	namespace editor
	{

class DependenciesAsDotWizardTool : public IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool launch(ui::Widget* parent, IEditor* editor, db::Group* group, db::Instance* instance) override final;
};

	}
}
