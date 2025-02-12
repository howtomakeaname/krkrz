#define NOMINMAX
#include "tjsCommHead.h"
#include "NullDrawDevice.h"
#include "BasicDrawDevice.h"
#include "OGLDrawDevice.h"

tTJSNativeClass* TVPCreateDefaultDrawDevice() 
{
//	return new tTJSNC_OGLDrawDevice();
	return new tTJSNC_BasicDrawDevice();
}
