#include "Editor/Pipeline/PipelineDependency.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependency", PipelineDependency, Object)

PipelineDependency::PipelineDependency()
:	pipelineHash(0)
,	sourceAssetHash(0)
,	flags(0)
,	reason(0)
,	parent(0)
{
}

	}
}
