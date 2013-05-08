#ifndef traktor_flash_ActionVM1_H
#define traktor_flash_ActionVM1_H

#include "Core/Timer/Timer.h"
#include "Flash/Action/IActionVM.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionVMTrace1;

/*! \brief ActionScript virtual machine.
 * \ingroup Flash
 *
 * Virtual machine to execute blocks of ActionScript
 * byte code.
 */
class T_DLLCLASS ActionVM1 : public IActionVM
{
	T_RTTI_CLASS;

public:
	ActionVM1();

	virtual Ref< const IActionVMImage > load(BitReader& br) const;

	virtual void execute(ActionFrame* frame) const;

	virtual void serialize(ISerializer& s);

private:
	Timer m_timer;
	Ref< ActionVMTrace1 > m_trace;
};

	}
}

#endif	// traktor_flash_ActionVM1_H
