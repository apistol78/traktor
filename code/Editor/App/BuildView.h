#ifndef traktor_editor_BuildView_H
#define traktor_editor_BuildView_H

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class BuildChartControl;

		}
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

	void endBuild(uint32_t core);

private:
	Ref< ui::custom::BuildChartControl > m_chartControl;
};

	}
}

#endif	// traktor_editor_BuildView_H
