#include "Flash/Action/Avm2/ActionVM2.h"
#include "Flash/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVM2", ActionVM2, IActionVM)

Ref< const IActionVMImage > ActionVM2::load(BitReader& br) const
{
	Ref< ActionVMImage2 > image = new ActionVMImage2();
	if (!image->m_abcFile.load(br))
		return 0;

	image->m_abcFile.dump();

	return image;
}

	}
}
