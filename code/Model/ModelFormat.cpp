#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelFormat", ModelFormat, Object)

Ref< Model > ModelFormat::readAny(const Path& filePath)
{
	return readAny(filePath, [&](const Path& p) -> Ref< IStream >{
		return FileSystem::getInstance().open(p, File::FmRead);
	});
}

Ref< Model > ModelFormat::readAny(const Path& filePath, const std::function< Ref< IStream >(const Path&) >& openStream)
{
	Ref< Model > md;

	TypeInfoSet formatTypes;
	type_of< ModelFormat >().findAllOf(formatTypes, false);

	for (const auto formatType : formatTypes)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >(formatType->createInstance());
		if (!modelFormat || !modelFormat->supportFormat(filePath.getExtension()))
			continue;

		md = modelFormat->read(filePath, openStream);
		if (md)
			break;
	}

	return md;
}

bool ModelFormat::writeAny(const Path& filePath, const Model* model)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmWrite);
	if (file)
		return writeAny(file, filePath.getExtension(), model);
	else
		return false;
}

bool ModelFormat::writeAny(IStream* stream, const std::wstring& extension, const Model* model)
{
	TypeInfoSet formatTypes;
	type_of< ModelFormat >().findAllOf(formatTypes);

	for (const auto formatType : formatTypes)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >(formatType->createInstance());
		if (!modelFormat)
			continue;

		std::wstring description;
		std::vector< std::wstring > extensions;
		modelFormat->getExtensions(description, extensions);

		for (const auto& ext : extensions)
		{
			if (compareIgnoreCase(extension, ext) == 0)
				return modelFormat->write(stream, model);
		}
	}

	return false;
}

	}
}
