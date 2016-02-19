#include "Flash/Action/Avm2/ActionVM2.h"
#include "Flash/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVM2", ActionVM2, IActionVM)

Ref< const IActionVMImage > ActionVM2::load(SwfReader& swf) const
{
	Ref< ActionVMImage2 > image = new ActionVMImage2();
	if (!image->m_abcFile.load(swf))
		return 0;

	image->m_abcFile.dump();

	return image;
}

	}
}
