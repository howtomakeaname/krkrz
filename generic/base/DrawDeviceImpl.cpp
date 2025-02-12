#define NOMINMAX
#include "tjsCommHead.h"
#include "Application.h"

tTJSNativeClass* TVPCreateDefaultDrawDevice() {
	return Application->CreateDrawDevice();
}
