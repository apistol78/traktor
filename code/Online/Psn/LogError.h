#ifndef traktor_online_LogError_H
#define traktor_online_LogError_H

namespace traktor
{

	namespace online
	{

class LogError
{

public:
	static void logErrorSaveData(int32_t err);
	static void logTrophyError(int32_t err);
	static void logBindErrorSaveData(uint32_t err);

};

	}
}

#endif	// traktor_online_LogError_H
