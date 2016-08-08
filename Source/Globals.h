#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define LOG(format, ...) _log(__FILE__, __LINE__, format, __VA_ARGS__)

void _log(const char file[], int line, const char* format, ...);

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846f
#define HAVE_M_PI

// New useful types
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

// Useful macros
#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

// Deletes a buffer
#define RELEASE( x )\
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

// Deletes an array of buffers
#define RELEASE_ARRAY( x )\
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }

// Configuration -----------
#define ASSETS_FOLDER "Assets"
#define VERSION "0.4-alpha"

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

// Disable STL exceptions
#define _HAS_EXCEPTIONS 0
#define _STATIC_CPPLIB

#endif // __GLOBALS_H__