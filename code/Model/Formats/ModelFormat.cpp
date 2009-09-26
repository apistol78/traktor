#include "Model/Formats/ModelFormat.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/StringUtilities.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelFormat", ModelFormat, Object)

Model* ModelFormat::readAny(const Path& filePath, uint32_t importFlags)
{
	Ref< model::Model > md;

	std::vector< const Type* > formatTypes;
	type_of< ModelFormat >().findAllOf(formatTypes);

	for (std::vector< const Type* >::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >((*i)->newInstance());
		if (!modelFormat)
			continue;

		if (!modelFormat->supportFormat(filePath))
			continue;

		md = modelFormat->read(filePath, importFlags);
		if (md)
			break;
	}

	return md;
}

bool ModelFormat::writeAny(const Path& filePath, const Model* model)
{
	std::vector< const Type* > formatTypes;
	type_of< ModelFormat >().findAllOf(formatTypes);

	for (std::vector< const Type* >::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >((*i)->newInstance());
		if (!modelFormat)
			continue;

		std::wstring description;
		std::vector< std::wstring > extensions;
		modelFormat->getExtensions(description, extensions);

		for (std::vector< std::wstring >::const_iterator i = extensions.begin(); i != extensions.end(); ++i)
		{
			if (compareIgnoreCase(filePath.getExtension(), *i) == 0)
				return modelFormat->write(filePath, model);
		}
	}

	return false;
}

	}
}
