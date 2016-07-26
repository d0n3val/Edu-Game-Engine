#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);

void log(const char file[], int line, const char* format, ...);

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define HAVE_M_PI

// New useful types
typedef unsigned int uint;
typedef unsigned char uchar;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

// Useful macros
#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

// Deletes a buffer
#define RELEASE( x ) \
    {									  \
       if( x != nullptr )   \
       {						      \
         delete x;                  \
	     x = nullptr;             \
       }                      \
    }

// Deletes an array of buffers
#define RELEASE_ARRAY( x ) \
	{                              \
       if( x != nullptr )              \
       {                            \
           delete[] x;                \
	       x = nullptr;                    \
		 }                            \
                              \
	 }

// Configuration -----------
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define SCREEN_SIZE 1
#define WIN_FULLSCREEN false
#define WIN_RESIZABLE true
#define WIN_BORDERLESS false
#define WIN_FULLSCREEN_DESKTOP false
#define VSYNC true
#define TITLE "EDU Engine"

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled

#endif // __GLOBALS_H__