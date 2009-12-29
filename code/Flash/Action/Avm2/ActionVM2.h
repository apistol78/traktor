#ifndef traktor_flash_ActionVM2_H
#define traktor_flash_ActionVM2_H

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

/*! \brief ActionScript virtual machine 2.
 * \ingroup Flash
 *
 * Virtual machine to execute blocks of ActionScript
 * byte code.
 */
class T_DLLCLASS ActionVM2 : public IActionVM
{
	T_RTTI_CLASS;

public:
	/*! \brief Execute frame.
	 *
	 * \param frame Execution frame.
	 */
	virtual void execute(ActionFrame* frame) const;
};

	}
}

#endif	// traktor_flash_ActionVM2_H
