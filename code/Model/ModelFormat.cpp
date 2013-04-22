#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelFormat", ModelFormat, Object)

Ref< Model > ModelFormat::readAny(const Path& filePath, uint32_t importFlags)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (file)
		return readAny(file, filePath.getExtension(), importFlags);
	else
		return 0;
}

Ref< Model > ModelFormat::readAny(IStream* stream, const std::wstring& extension, uint32_t importFlags)
{
	Ref< Model > md;

	TypeInfoSet formatTypes;
	type_of< ModelFormat >().findAllOf(formatTypes, false);

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >((*i)->createInstance());
		if (!modelFormat)
			continue;

		if (!modelFormat->supportFormat(extension))
			continue;

		md = modelFormat->read(stream, importFlags);
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

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >((*i)->createInstance());
		if (!modelFormat)
			continue;

		std::wstring description;
		std::vector< std::wstring > extensions;
		modelFormat->getExtensions(description, extensions);

		for (std::vector< std::wstring >::const_iterator i = extensions.begin(); i != extensions.end(); ++i)
		{
			if (compareIgnoreCase(extension, *i) == 0)
				return modelFormat->write(stream, model);
		}
	}

	return false;
}

	}
}
