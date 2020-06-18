#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Mutex.h"
#include "Model/Model.h"
#include "Model/Formats/Blend/ModelFormatBlend.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

Mutex s_globalLock(Guid(L"{672E3E2D-14A8-4B07-A18F-27C30FD5B43D}"));

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatBlend", 0, ModelFormatBlend, ModelFormat)

void ModelFormatBlend::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Blender";
	outExtensions.push_back(L"blend");
}

bool ModelFormatBlend::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"blend") == 0;
}

Ref< Model > ModelFormatBlend::read(const Path& filePath, const std::wstring& filter, const std::function< Ref< IStream >(const Path&) >& openStream) const
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(s_globalLock);

	// Determine working path.
	std::wstring scratchPath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Blender";
	if (!FileSystem::getInstance().makeAllDirectories(scratchPath))
		return nullptr;

	// Copy source model into working path.
	Ref< IStream > file = openStream(filePath);
	if (!file)
		return nullptr;

	Ref< IStream > tmpFile = FileSystem::getInstance().open(scratchPath + L"/__source__.blend", File::FmWrite);
	if (!tmpFile)
		return nullptr;

	if (!StreamCopy(tmpFile, file).execute())
		return nullptr;

	tmpFile->close();
	tmpFile = nullptr;

	// Create export script.
	file = FileSystem::getInstance().open(scratchPath + L"/__export__.py", File::FmWrite);
	if (!file)
		return nullptr;

	FileOutputStream os(file, new Utf8Encoding());
	os << L"import bpy" << Endl;
	os << L"bpy.ops.export_scene.fbx(" << Endl;
	os << L"	filepath=\"" << scratchPath << L"/__intermediate__.fbx\"," << Endl;
	os << L"	axis_forward=\"Z\"," << Endl;
	os << L"	axis_up=\"Y\"," << Endl;
	os << L"	use_selection=False," << Endl;
	os << L"	global_scale=0.01" << Endl;
	os << L")" << Endl;
	os.close();

	file = nullptr;

	// Execute export script through headless blender process.
#if defined(_WIN32)
	std::wstring blender = L"c:\\Program Files\\Blender Foundation\\Blender 2.81\\blender.exe";
#elif defined(__LINUX__)
	std::wstring blender = L"/home/apistol/blender-2.81-115a5bf65a6b-linux-glibc217-x86_64/blender";
#else
	std::wstring blender = L"blender";
#endif
	std::wstring commandLine = L"\"" + blender + L"\" -b " + scratchPath + L"/__source__.blend -P " + scratchPath + L"/__export__.py";
	Ref< IProcess > process = OS::getInstance().execute(
		commandLine,
		scratchPath,
		nullptr,
		OS::EfNone
	);
	if (!process)
		return nullptr;
	if (!process->wait())
		return nullptr;
	if (process->exitCode() != 0)
		return nullptr;

	// Import intermediate model.
	return ModelFormat::readAny(scratchPath + L"/__intermediate__.fbx", filter);
}

bool ModelFormatBlend::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
