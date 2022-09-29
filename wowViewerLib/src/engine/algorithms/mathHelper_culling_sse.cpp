#include "mathHelper.h"
#include "mathHelper_culling_sse.h"

#if (__AVX__ && __SSE2__)
#include <emmintrin.h>
#include <immintrin.h>
#endif


