#include "Editor/PipelineDependency.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependency", PipelineDependency, Object)

PipelineDependency::PipelineDependency()
:	pipelineHash(0)
,	sourceAssetHash(0)
,	build(false)
,	reason(0)
,	parent(0)
{
}

	}
}
