ifeq ($(VCPKG_ROOT),)
$(error Variables VCPKG_ROOT not set correctly.)
endif

ifeq ($(shell type cygpath > /dev/null && echo true),true)
FIXPATH = cygpath -ma
else
FIXPATH = realpath
endif

# Detect OS and set default PRESET accordingly
PRESET?=x64-windows
BUILD_TYPE?=Release
CMAKEOPT?="-DUSE_SJIS=ON"

ifeq ($(DATAPATH),)
DATAPATH=data
endif

DATAPATH_ABS=$(shell $(FIXPATH) "$(DATAPATH)")

BUILD_PATH=$(shell cmake --preset $(PRESET) -N | grep BUILD_DIR | sed 's/.*BUILD_DIR="\(.*\)"/\1/')

INSTALL_PREFIX?=install

.PHONY: build  prebuild

all: build

# cmake 処理実行
# CMAKEOPT で引数定義追加
prebuild:
	cmake --preset $(PRESET) ${CMAKEOPT}
# ビルド実行
build:
	cmake --build $(BUILD_PATH) --config $(BUILD_TYPE)

clean:
	cmake --build $(BUILD_PATH) --config $(BUILD_TYPE) --target clean

install:
	cmake --install $(BUILD_PATH) --config $(BUILD_TYPE) --prefix $(INSTALL_PREFIX)

# WIN版用ルール
ifeq (windows,$(findstring windows,$(PRESET)))

ifeq (x86,$(findstring x86,$(PRESET)))
PLUGINS_SRC_DIR=plugin
PLUGINS_DST_DIR=$(BUILD_PATH)/$(BUILD_TYPE)/plugin
EXEFILE=$(BUILD_PATH)/$(BUILD_TYPE)/krkrz.exe
endif

ifeq (x64,$(findstring x64,$(PRESET)))
PLUGINS_SRC_DIR=plugin64
PLUGINS_DST_DIR=$(BUILD_PATH)/$(BUILD_TYPE)/plugin64
EXEFILE=$(BUILD_PATH)/$(BUILD_TYPE)/krkrz64.exe
endif

PLUGINS = $(patsubst $(PLUGINS_SRC_DIR)/%.dll, $(PLUGINS_DST_DIR)/%.dll, $(wildcard $(PLUGINS_SRC_DIR)/*.dll))

$(PLUGINS_DST_DIR)/%.dll : $(PLUGINS_SRC_DIR)/%.dll
	@mkdir -p `dirname $@`
	cp $< $@

$(EXEFILE): build

run: $(EXEFILE) $(PLUGINS)
	$(EXEFILE) $(DATAPATH_ABS)

endif
