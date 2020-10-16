#ifndef __LEAKS_H__
#define __LEAKS_H__

#ifdef _DEBUG
#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
//allocations to be of _CLIENT_BLOCK type
#else
#define MYDEBUG_NEW
#endif // _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new MYDEBUG_NEW
#endif

#endif /* __LEAKS_H__ */
