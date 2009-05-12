#include "Model/Import/ImportFormat.h"
#include "Core/Heap/Ref.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ImportFormat", ImportFormat, Object)

model::Model* ImportFormat::importAny(const Path& filePath, uint32_t importFlags)
{
	Ref< model::Model > md;

	std::vector< const Type* > importerTypes;
	type_of< ImportFormat >().findAllOf(importerTypes);

	for (std::vector< const Type* >::iterator i = importerTypes.begin(); i != importerTypes.end(); ++i)
	{
		Ref< ImportFormat > importFormat = dynamic_type_cast< ImportFormat* >((*i)->newInstance());
		if (!importFormat)
			continue;

		if (!importFormat->supportFormat(filePath))
			continue;

		md = importFormat->import(filePath, importFlags);
		if (md)
			break;
	}

	return md;
}

	}
}
