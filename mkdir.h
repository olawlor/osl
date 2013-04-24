/**
  Portably make a new directory.
Orion Sky Lawlor, olawlor@acm.org, 2006/3/2 (Public Domain)
*/
#ifndef __OSL_MKDIR_H
#define __OSL_MKDIR_H

#ifdef WIN32
#include <windows.h>
namespace osl {
  inline bool mkdir(const char *pathname) { 
	return 0==CreateDirectory(pathname,0);
  }
};
#else /* UNIX-like system */
#include <sys/stat.h>
namespace osl {
  inline bool mkdir(const char *pathname) { 
	return 0==::mkdir(pathname,0777);
  }
};
#endif


#endif
