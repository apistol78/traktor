#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor
{
	namespace sound
	{

class BankControlGrain;

class BankControl : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(BankControlGrain* item);

	void removeAll();

	BankControlGrain* getSelected() const;

	const RefArray< BankControlGrain >& getGrains() const;

protected:
	virtual void layoutCells(const ui::Rect& rc) override final;

private:
	RefArray< BankControlGrain > m_cells;
};

	}
}

