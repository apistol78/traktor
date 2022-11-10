/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

