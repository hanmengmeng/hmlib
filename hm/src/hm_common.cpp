#include "hm_common.h"

namespace hm
{

void BreakDebugger() {
    __debugbreak();
#ifndef _DEBUG
    _exit(1);
#endif
}


}