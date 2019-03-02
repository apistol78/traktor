#pragma once

#include "Flash/Action/IActionVMImage.h"
#include "Flash/Action/Avm2/AbcFile.h"

namespace traktor
{
	namespace flash
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

