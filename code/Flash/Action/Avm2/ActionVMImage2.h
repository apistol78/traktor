#ifndef traktor_flash_ActionVMImage2_H
#define traktor_flash_ActionVMImage2_H

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
	virtual void execute(ActionFrame* frame) const;

	virtual void serialize(ISerializer& s);

private:
	friend class ActionVM2;

	AbcFile m_abcFile;
};

	}
}

#endif	// traktor_flash_ActionVMImage2_H
