#include "Core/Log/Log.h"
#include "Amalgam/Run/Impl/LibraryHelper.h"

namespace traktor
{
	namespace amalgam
	{

Ref< Object > loadAndInstantiate(const std::wstring& typeName)
{
	const TypeInfo* type = TypeInfo::find(typeName);
	if (!type)
	{
		log::error << L"Unable to instantiate \"" << typeName << L"\"; no such type" << Endl;
		return 0;
	}

	Ref< Object > object = dynamic_type_cast< Object* >(type->createInstance());
	if (!object)
	{
		log::error << L"Unable to instantiate \"" << typeName << L"\"; type abstract" << Endl;
		return 0;
	}

	return object;
}

	}
}
