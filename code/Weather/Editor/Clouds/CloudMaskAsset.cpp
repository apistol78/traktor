#include "Weather/Editor/Clouds/CloudMaskAsset.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.weather.CloudMaskAsset", CloudMaskAsset, editor::Asset)

const Type* CloudMaskAsset::getOutputType() const
{
	return &type_of< CloudMaskResource >();
}

	}
}
