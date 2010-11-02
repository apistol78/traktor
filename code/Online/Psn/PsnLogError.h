#ifndef traktor_online_PsnLogError_H
#define traktor_online_PsnLogError_H

namespace traktor
{

	namespace online
	{

class PsnLogError
{
public:
	static std::wstring getSaveDataErrorString(int32_t err);

	static std::wstring getTrophyErrorString(int32_t err);

	static std::wstring getSaveDataBindErrorString(uint32_t err);

};

	}
}

#endif	// traktor_online_PsnLogError_H
