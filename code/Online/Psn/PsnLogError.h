/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
