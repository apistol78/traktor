#include "Spark/Action/Avm2/ActionVM2.h"
#include "Spark/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionVM2", ActionVM2, IActionVM)

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
