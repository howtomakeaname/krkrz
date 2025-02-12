//---------------------------------------------------------------------------
// TJS2 Script Managing
//---------------------------------------------------------------------------
#include "tjsCommHead.h"
#include <memory>
#include "Application.h"
#include "MsgIntf.h"
#include "CharacterSet.h"

//---------------------------------------------------------------------------
// Hash Map Object を書き出すためのサブプロセスとして起動しているかどうか
// チェックする
// Windows 以外では、ないものとして扱ってもいいか
bool TVPCheckProcessLog() { return false; }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Script system initialization script
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ttstr TVPGetSystemInitializeScript()
{
	// read system init file from resource

	const tjs_char *filename = TJS_W("SysInitScript.tjs");
	tjs_uint64 flen;
	auto buf = Application->ReadResource(filename, &flen);
	if( buf.get() == nullptr ) {
		TVPThrowExceptionMessage(TVPCannotOpenStorage, ttstr(filename));
	}

	// XXX BOMはいってるとだめっぽい？
	tjs_string ret((tjs_char*)(buf.get())+1, flen/2-1);
	//Application->PrintConsole(ret.c_str(), ret.size());

	return ttstr(ret.c_str());
}
//---------------------------------------------------------------------------
