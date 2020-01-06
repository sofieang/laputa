#ifndef __PREFIX_H__
#define __PREFIX_H__

// Prefix file - anything that is platform-specific and which concerns compilation goes here

#ifdef _WINDOWS
// turn off warnings about conversion to float
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif


#ifdef __APPLE__
#define HAVE_INLINE 1
#endif

#define USE_DOUBLE_FLOAT

// float and int formats
#ifdef USE_DOUBLE_FLOAT
#define t_float double
#define QueryFloatAttribute QueryDoubleAttribute
#else
#define t_float float
#endif
#define t_int int

#include "mempool.h"
#define MEMORY_ALLOCATOR_LINK Moya::Allocator<pair<const unsigned t_int, Link>>

#ifdef _WINDOWS
#define DATA_PATH_PRIMARY "data/"
#define DOCS_PATH_PRIMARY "docs/"
#endif

#ifdef __APPLE__
#define DATA_PATH_PRIMARY "/Contents/Resources/data/"
#define DOCS_PATH_PRIMARY "/Contents/Resources/docs/"
#endif

#ifdef __linux__
#define DATA_PATH_PRIMARY "/usr/local/share/laputa/data/"
#define DOCS_PATH_PRIMARY "/usr/local/share/laputa/docs/"
#define DATA_PATH_SECONDARY "data/"
#define DOCS_PATH_SECONDARY "docs/"
#endif

#endif
