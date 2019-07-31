#pragma once

#include "Spark/Action/IActionVMImage.h"
#include "Spark/Action/Avm2/AbcFile.h"

namespace traktor
{
	namespace spark
	{

class ActionVMImage2 : public IActionVMImage
{
	T_RTTI_CLASS;

public:
	virtual void execute(ActionFrame* frame) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ActionVM2;

	AbcFile m_abcFile;
};

	}
}

