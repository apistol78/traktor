/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Endian_H
#define traktor_Endian_H

#include <utility>

namespace traktor
{

/*! \ingroup Core */
//@{

template < typename T >
inline void swap8in16(T& value)
{
	uint8_t* p = reinterpret_cast< uint8_t* >(&value);
	switch (sizeof(T))
	{
	case 1:
		break;
	case 2:
		std::swap(p[0], p[1]);
		break;
	case 4:
		std::swap(p[0], p[1]);
		std::swap(p[2], p[3]);
		break;
	case 8:
		std::swap(p[0], p[1]);
		std::swap(p[2], p[3]);
		std::swap(p[4], p[5]);
		std::swap(p[6], p[7]);
		break;
	}
}

template < typename T >
inline void swap8in32(T& value)
{
	uint8_t* p = reinterpret_cast< uint8_t* >(&value);
	switch (sizeof(T))
	{
	case 1:
		break;
	case 2:
		std::swap(p[0], p[1]);
		break;
	case 4:
		std::swap(p[0], p[3]);
		std::swap(p[1], p[2]);
		break;
	case 8:
		std::swap(p[0], p[3]);
		std::swap(p[1], p[2]);
		std::swap(p[4], p[7]);
		std::swap(p[5], p[6]);
		break;
	}
}

template < typename T >
inline void swap8in64(T& value)
{
	uint8_t* p = reinterpret_cast< uint8_t* >(&value);
	switch (sizeof(T))
	{
	case 1:
		break;
	case 2:
		std::swap(p[0], p[1]);
		break;
	case 4:
		std::swap(p[0], p[3]);
		std::swap(p[1], p[2]);
		break;
	case 8:
		std::swap(p[0], p[7]);
		std::swap(p[1], p[6]);
		std::swap(p[2], p[5]);
		std::swap(p[3], p[4]);
		break;
	}
}

//@}

}

#endif	// traktor_Endian_H
