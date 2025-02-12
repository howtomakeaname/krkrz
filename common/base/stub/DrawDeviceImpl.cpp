#define NOMINMAX
#include "tjsCommHead.h"
#include "NullDrawDevice.h"

tTJSNativeClass* TVPCreateDefaultDrawDevice() {
    return new tTJSNC_NullDrawDevice();
}
