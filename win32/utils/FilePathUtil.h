
#ifndef __FILE_PATH_UTIL_H__
#define __FILE_PATH_UTIL_H__

#include <string>
#include <stdlib.h>

#include <shlwapi.h>
#include <winnetwk.h>


inline tjs_string IncludeTrailingBackslash( const tjs_string& path ) {
	if( path[path.length()-1] != TJS_W('\\') ) {
		return tjs_string(path+TJS_W("\\"));
	}
	return tjs_string(path);
}
inline tjs_string ExcludeTrailingBackslash( const tjs_string& path ) {
	if( path[path.length()-1] == TJS_W('\\') ) {
		return tjs_string(path.c_str(),path.length()-1);
	}
	return tjs_string(path);
}
// 末尾の /\ は含まない
inline tjs_string ExtractFileDir( const tjs_string& path ) {
	tjs_char drive[_MAX_DRIVE];
	tjs_char dir[_MAX_DIR];
	_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	tjs_string dirstr = tjs_string( dir );
	if( dirstr[dirstr.length()-1] != TJS_W('\\') ) {
		return tjs_string( drive ) + dirstr;
	} else {
		return tjs_string( drive ) + dirstr.substr(0,dirstr.length()-1);
	}
}
// 末尾の /\ を含む
inline tjs_string ExtractFilePath( const tjs_string& path ) {
	tjs_char drive[_MAX_DRIVE];
	tjs_char dir[_MAX_DIR];
	_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir,_MAX_DIR, NULL, 0, NULL, 0 );
	return tjs_string( drive ) + tjs_string( dir );
}

inline bool DirectoryExists( const tjs_string& path ) {
	return (0!=::PathIsDirectory(path.c_str()));
}

inline bool FileExists( const tjs_string& path ) {
	return ( (0!=::PathFileExists(path.c_str())) && (0==::PathIsDirectory(path.c_str())) );
	return false;
}

inline tjs_string ChangeFileExt( const tjs_string& path, const tjs_string& ext ) {
	tjs_char drive[_MAX_DRIVE];
	tjs_char dir[_MAX_DIR];
	tjs_char fname[_MAX_FNAME];
	_wsplitpath_s( path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, NULL, 0 );
	return tjs_string( drive ) + tjs_string( dir ) + tjs_string( fname ) + ext;
}
inline tjs_string ExtractFileName( const tjs_string& path ) {
	tjs_char fname[_MAX_FNAME];
	tjs_char ext[_MAX_EXT];
	_wsplitpath_s( path.c_str(), NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT );
	return tjs_string( fname ) + tjs_string( ext );
}
inline tjs_string ExtractFileExt( const tjs_string& path ) {
	tjs_char ext[_MAX_EXT];
	_wsplitpath_s( path.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT );
	return tjs_string( ext );
}
inline tjs_string ExpandUNCFileName( const tjs_string& path ) {
	tjs_string result;
	DWORD InfoSize = 0;
	if( ERROR_MORE_DATA == WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, NULL, &InfoSize) ) {
		UNIVERSAL_NAME_INFO* pInfo = reinterpret_cast<UNIVERSAL_NAME_INFO*>( ::GlobalAlloc(GMEM_FIXED, InfoSize) );
		DWORD ret = ::WNetGetUniversalName( path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, pInfo, &InfoSize);
		if( NO_ERROR == ret ) {
			result = tjs_string(pInfo->lpUniversalName);
		}
		::GlobalFree(pInfo);
	} else {
		tjs_char fullpath[_MAX_PATH];
		result = tjs_string( _wfullpath( fullpath, path.c_str(), _MAX_PATH ) );
	}
	return result;
}

#endif // __FILE_PATH_UTIL_H__
