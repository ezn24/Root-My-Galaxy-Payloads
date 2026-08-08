#define BUILD_FINGERPRINT \
  "samsung/e3qzhx/e3q:14/UP1A.231005.007/S9280ZHS6DZG1:user/release-keys"

#include "targets/e3q-S928USQS6DZF2/target.h"

#undef BUILD_VARIANT_LABEL
#if defined(APP_PAYLOAD) && APP_PAYLOAD
#define BUILD_VARIANT_LABEL "e3q-S9280ZHS6DZG1-app-physical-p0-oracle"
#else
#define BUILD_VARIANT_LABEL "e3q-S9280ZHS6DZG1-root-umh"
#endif

#undef P0_FINGERPRINT_HEADER
#define P0_FINGERPRINT_HEADER \
  "targets/e3q-S9280ZHS6DZG1/p0_fingerprint.h"

#if defined(APP_PAYLOAD) && APP_PAYLOAD
/* A5 iteration: try P0_ORACLE_PROBE_OFFSET=0x0 (gate at slide base
 * itself) to see if 0x100000 was the wrong slide interior page.  The
 * original DZF2 default 0x1f0000 was the slide boundary, 0x100000 was
 * the slide interior; both miss the gate on DZG1 in A4.  Trying 0
 * to see if the gate should be at the kernel text base itself. */
#undef P0_ORACLE_PROBE_OFFSET
#define P0_ORACLE_PROBE_OFFSET 0x0ULL
#endif
