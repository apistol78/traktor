#include <cstdlib>
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Editor/App/CheckSerializableTool.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.CheckSerializableTool", 0, CheckSerializableTool, IEditorTool)

std::wstring CheckSerializableTool::getDescription() const
{
	return i18n::Text(L"EDITOR_CHECK_SERIALIZABLE_CLASSES");
}

Ref< ui::IBitmap > CheckSerializableTool::getIcon() const
{
	return 0;
}

bool CheckSerializableTool::launch(ui::Widget* parent, IEditor* editor)
{
	AutoArrayPtr< uint8_t, AllocFreeAlign > memory((uint8_t*)Alloc::acquireAlign(1024 * 1024, 16, T_FILE_LINE));

	TypeInfoSet types;
	type_of< ISerializable >().findAllOf(types, false);

	for (TypeInfoSet::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		if (!(*i)->isInstantiable())
			continue;

		// Randomize memory.
		for (uint32_t j = 0; j < 1024 * 1024; ++j)
			memory[j] = std::rand() & 255;

		ISerializable* object0 = checked_type_cast< ISerializable* >((*i)->createInstance(memory.ptr()));
		if (!object0)
			continue;

		uint32_t hash0 = DeepHash(object0).get();

		// Randomize memory again.
		for (uint32_t j = 0; j < 1024 * 1024; ++j)
			memory[j] = std::rand() & 255;

		ISerializable* object1 = checked_type_cast< ISerializable* >((*i)->createInstance(memory.ptr()));
		if (!object1)
			continue;

		uint32_t hash1 = DeepHash(object1).get();

		// If all members are properly initialized then the hashes must match.
		if (hash0 != hash1)
			log::error << L"Class \"" << (*i)->getName() << L"\" not valid; uninitialized members not allowed" << Endl;
		else
			log::info << L"Class \"" << (*i)->getName() << L"\" ok" << Endl;
	}

	log::info << L"Done" << Endl;
	return true;
}

	}
}
