#include "Weather/Editor/Clouds/CloudMaskAsset.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.CloudMaskAsset", 0, CloudMaskAsset, editor::Asset)

const TypeInfo* CloudMaskAsset::getOutputType() const
{
	return &type_of< CloudMaskResource >();
}

	}
}
