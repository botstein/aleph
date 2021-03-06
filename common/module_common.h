
#ifndef _ALEPH_MODULE_H_
#define _ALEPH_MODULE_H_

#include "types.h"

// synchronize with protocol
#define MODULE_NAME_LEN 24

// module version
typedef struct _moduleVersion_t {
  u8 maj;       // major version (fundamentally different)
  u8 min;       // minor version (may not be i/o compatibeble)
  u16 revision; // revision no.  (compatible)
} moduleVersion_t;

#endif

