#define NOMINMAX
#include "tjsCommHead.h"
#include "Application.h"

tTJSNativeClass* TVPGetDefaultDrawDevice() {
	return Application->GetDefaultDrawDevice();
}
