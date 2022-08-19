#pragma once

/*! \ingroup Drawing */
//@{

#if defined(_MSC_VER)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
#	define DRAWING_INCLUDE_EXR
#elif defined(__APPLE__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
#	if !defined(__IOS__)
#	    define DRAWING_INCLUDE_EXR
#   endif
#elif defined(__GNUC__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
#	if !defined(__ANDROID__)
#		define DRAWING_INCLUDE_EXR
#	endif
#endif

// Santiy check
#if !defined(DRAWING_INCLUDE_PNG) && defined(DRAWING_INCLUDE_ICO)
#	pragma message("DRAWING_INCLUDE_ICO require DRAWING_INCLUDE_PNG")
#	undef DRAWING_INCLUDE_ICO
#endif

#if defined(_DEBUG)
#	define DRAWING_CHECK_DATA
#endif

//@}

