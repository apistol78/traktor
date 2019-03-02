#pragma once

#include "Editor/IPipelineBuilder.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class BuildChartControl;

	}

	namespace editor
	{

class BuildView : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void beginBuild();

	void endBuild();

	void beginBuild(uint32_t core, const std::wstring& name);

	void endBuild(uint32_t core, IPipelineBuilder::BuildResult result);

private:
	Ref< ui::BuildChartControl > m_chartControl;
};

	}
}

