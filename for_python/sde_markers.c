#include "sde_markers.h"
#ifdef __cplusplus
extern "C" {
#endif

void sde_start_marker(void)
{
    __SSC_MARK(0xFACE);
}

void sde_stop_marker(void)
{
    __SSC_MARK(0xDEAD);
}

#ifdef __cplusplus
}
#endif
