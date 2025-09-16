#include "tjsCommHead.h"
#include "OGLApplication.h"

OGLApplication::OGLApplication() 
: tTVPApplication() 
{
}

#include "OGLDrawDevice.h"

// アプリ処理用の DrawDevice実装を返す
tTJSNativeClass* 
OGLApplication::GetDefaultDrawDevice()
{
    static tTJSNativeClass* draw_device = nullptr;
    if (draw_device == nullptr) {
        draw_device = new tTJSNC_OGLDrawDevice();
    }
    return draw_device;
}
