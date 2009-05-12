#ifndef traktor_drawing_Config_H
#define traktor_drawing_Config_H

/*! \ingroup Drawing */
//@{

#if defined(_MSC_VER)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
//#	define DRAWING_INCLUDE_EXR
#elif defined(__APPLE__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
//#	define DRAWING_INCLUDE_EXR
#else defined(__GNUC__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
//#	define DRAWING_INCLUDE_EXR
#endif

//@}

#endif	// traktor_drawing_Config_H
