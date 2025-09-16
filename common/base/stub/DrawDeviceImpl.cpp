#define NOMINMAX
#include "tjsCommHead.h"
#include "NullDrawDevice.h"

static tTJSNativeClass *nativeClass = NULL;

tTJSNativeClass* TVPGetDefaultDrawDevice() {
    if (!nativeClass) {
        nativeClass = new tTJSNC_NullDrawDevice();
    }
    return nativeClass;
}
