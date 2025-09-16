# 吉里吉里Z multi platform

## 概要

マルチプラットフォーム展開を想定した吉里吉里Zです

・OpenGLベース描画機構を持ちます Canvas/Screen/Texture/Shader
・極力外部ライブラリを参照する形で構築されています。

外部ライブラリの参照には vcpkg を利用しています。

このレポジトリには以下の構成が含まれています

1. 機種依存部をなるべく削除したライブラリ版吉里吉里Z
2. 従来のWIN32版準拠の吉里吉里Z
3. OpenGL 拡張用のコード

※ Windows以外の環境用のバイナリ生成には別途 krkrz_sdl / krkrz_glfw のレポジトリを利用して下さい

## 開発環境準備

### msys2

msys2 をインストールして基礎開発ツールを導入しておきます

```bash
pacman -S base-devel
```

作業の補助に使います（主に make を利用）

### コンパイル環境

Windows用に Visual Studio をインストールして
C++ コンパイラを使える状態にしておきます。

あわせて Visual Studio 付属の Cmake / Ninja を利用します。

Visual Studio 2022 以降は vcpkg があわせて導入されますが、
自前k環境を使う場合は競合してまうので入れない様にして下さい。

アセンブリの処理用に nasm 2.10.09 が必要です。

### vcpkg 環境準備

vcpkg を導入します

https://learn.microsoft.com/ja-jp/vcpkg/get_started/overview

vcpkg のフォルダを環境変数 VCPKG_ROOT に設定しておきます。

```bash
# dos
set VCPKG_ROOT="c:\work\vcpkg"

# msys/cygwin
export VCPKG_ROOT='c:\work\vcpkg'
```

## ビルド

### ソースのチェックアウト

git clone 後 submodule 更新しておいてください

```
git submodule update --init
```

### ビルド

ビルドに必要な定義が行われた Makefile が準備されていいます。

CMakePresets.json 中のプリセット定義をつかってビルドします。
必要なライブラリは vcpkg.json によってセットアップされます。


```bash
# 構築対象 preset設定（未定義時は x86-windows）
# CMakePresets.json 
export PRESET=x86-windows

# ビルドタイプ指定（未定義時は Release）
export BUILD_TYPE=Master
#export BUILD_TYPE=Develop
#export BUILD_TYPE=Debug

# cmake オプション指定
# BUILD_LIB ライブラリ版を構成する
# USE_SJIS  デフォルトをSJIS(MBSC) にする
# USE_OPENGL OPELGL系機能を使う（起動時に GLESのdllが必要になる）
export CMAKEOPT="-DUSE_SJIS=ON"

# cmake プロジェクト生成
# この段階で vcpkg が処理されてライブラリが準備されます
make prebuild

# cmake でビルド
make build 

# インストール処理
INSTALL_PREFIX=install make install

```	

処理内容詳細は Makefile と CMakeList.txt を参照して下さい。

### テスト実行

Makefile にそのままトップフォルダで実行可能なルールが定義されています。

```bash
# cmake 経由で実行
make run
```

OpenGL 動作時は以下のファイル構成が必要になります

    plugin/ プラグインフォルダ
      libEGL.dll        OpenGL の egl用DLL
      libGLESv2.dll     OpenGL の GLES2用DLL
    plugin64/ プラグインフォルダ 64bit
      libEGL.dll        OpenGL の egl用DLL
      libGLESv2.dll     OpenGL の GLES2用DLL

## デバッグ実行

### VisualStudio でのデバッグ

以下の手順でソースデバッグできます

- Visual Studio を起動して、プロジェクトなしの状態のウインドウに実行ファイルをドロップする
- デバッグのプロパティの作業フォルダにプロジェクトフォルダを指定（プラグインフォルダの参照先になるため）
- デバッグのプロパティの引数に data フォルダの場所をフルパスで指定（現行仕様がexe相対もしくは絶対パス）

### VSCode でのデバッグ

次のような launch.jsonを準備して下さい。
program 部分に生成される実行ファイルのパス名を直接記載します。
args で処理対象フォルダを指定できます（フルパスになるように記載して下さい）

launch.json

```json
{
    // IntelliSense を使用して利用可能な属性を学べます。
    // 既存の属性の説明をホバーして表示します。
    // 詳細情報は次を確認してください: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "WINデバッグ起動",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "build/x86-windows/Debug/krkrz.exe",
            "args": ["${workspaceFolder}/data"],
            "stopAtEntry": false,
            "console":"externalTerminal",
            "cwd": "${workspaceFolder}",
            "environment": []
        }
    ]
}
```

# その他情報

tvpsnd_ia32
nasm 2.10.09 が必要です。

tvpgl_ia32
nasm 2.10.09 が必要です。

自動生成ファイル
吉里吉里Z本体にはいくつかの自動生成ファイルが存在します。
自動生成ファイルは直接編集せず、生成元のファイルを編集します。
生成には主にbatファイルとperlが使用されているので、perlのインストールが必要です。
各生成ファイルを左に ':' 以降に生成元ファイルを列挙します。

tjs2/syntax/compile.bat で以下のファイルが生成されます。

tjs.tab.cpp/tjs.tab.hpp : tjs.y
tjsdate.tab.cpp/tjsdate.tab.hpp : tjsdate.y
tjspp.tab.cpp/tjspp.tab.hpp : tjspp.y
tjsDateWordMap.cc : gen_wordtable.bat

これらのファイルの生成には bison が必要です。
bison には libiconv2.dll libintl3.dll regex2.dll が必要なので一緒にインストールする必要があります。
http://gnuwin32.sourceforge.net/packages/bison.htm
http://gnuwin32.sourceforge.net/packages/libintl.htm
http://gnuwin32.sourceforge.net/packages/libiconv.htm
http://gnuwin32.sourceforge.net/packages/regex.htm

visual/glgen/gengl.bat で以下のファイルが生成されます。
tvpgl.c/tvpgl.h : maketab.c/tvpps.c

visual/IA32/compile.bat で以下のファイルが生成されます。
tvpgl_ia32_intf.c/tvpgl_ia32_intf.h : *.nas

base/win32/makestub.bat で以下のファイルが生成されます。
FuncStubs.cpp/FuncStubs.h : makestub.pl内で指定されたヘッダーファイル内のTJS_EXP_FUNC_DEF/TVP_GL_FUNC_PTR_EXTERN_DECLマクロで記述された関数
tp_stub.cpp/tp_stub.h : 同上

