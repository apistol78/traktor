#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/System/OS.h"
#include "Update/PostLaunch.h"

namespace traktor
{
	namespace update
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.update.PostLaunch", 0, PostLaunch, IPostAction)

bool PostLaunch::execute() const
{
	return OS::getInstance().execute(
		m_fileName,
		m_arguments,
		L"",
		0,
		false,
		false,
		true
	) != 0;
}

bool PostLaunch::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"fileName", m_fileName);
	s >> Member< std::wstring >(L"arguments", m_arguments);
	return true;
}

	}
}
