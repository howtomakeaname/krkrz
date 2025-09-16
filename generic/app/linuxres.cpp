#include "tjsCommHead.h"
#include "CharacterSet.h"
#include "StorageIntf.h"
#include "DebugIntf.h"
#include "UtilStreams.h"
#include "SysInitIntf.h"

#include <string>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include <limits.h>

#define MEDIA_NAME TJS_W("resource")
#define SECTION_PREFIX "resource"

class tTVPResourceStorageMedia : public iTVPStorageMedia
{
	typedef tTVPResourceStorageMedia Self;
	static Self *Instance;

	tjs_int RefCount;
	void *mapped_memory;
	size_t mapped_size;
	
	struct ResourceEntry {
		const tjs_uint8* data;
		tjs_uint size;
	};
	
	std::map<std::string, ResourceEntry> resources;
	
	bool LoadResourceFromExecutable() {
		// 自分自身の実行ファイルのパスを取得

		char exe_path[PATH_MAX];
		ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
		if (len == -1) {
			TVPAddLog(TJS_W("Failed to get executable path"));
			return false;
		}
		exe_path[len] = '\0';
		
		// 実行ファイルを開く
		int fd = open(exe_path, O_RDONLY);
		if (fd == -1) {
			TVPAddLog(TJS_W("Failed to open executable file"));
			return false;
		}
		
		// ファイルサイズを取得
		struct stat st;
		if (fstat(fd, &st) == -1) {
			close(fd);
			TVPAddLog(TJS_W("Failed to get file size"));
			return false;
		}
		mapped_size = st.st_size;
		
		// ファイルをメモリにマップ
		mapped_memory = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
		close(fd);
		
		if (mapped_memory == MAP_FAILED) {
			TVPAddLog(TJS_W("Failed to mmap executable file"));
			mapped_memory = nullptr;
			return false;
		}
		
		// ELFヘッダーを取得
		Elf64_Ehdr* ehdr = static_cast<Elf64_Ehdr*>(mapped_memory);
		
		// ELFマジック番号を確認
		if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
			TVPAddLog(TJS_W("Not a valid ELF file"));
			munmap(mapped_memory, mapped_size);
			mapped_memory = nullptr;
			return false;
		}
		
		// セクションヘッダーテーブルの位置を取得
		Elf64_Shdr* shdr = reinterpret_cast<Elf64_Shdr*>(
			reinterpret_cast<char*>(mapped_memory) + ehdr->e_shoff);
		
		// セクション名テーブルのインデックス
		int shstrndx = ehdr->e_shstrndx;
		
		// セクション名テーブルのヘッダーを取得
		Elf64_Shdr* sh_strtab = &shdr[shstrndx];
		
		// セクション名テーブルの開始位置
		const char* strtab = reinterpret_cast<const char*>(mapped_memory) + sh_strtab->sh_offset;
		
		// 各セクションをチェック
		size_t prefix_len = strlen(SECTION_PREFIX);
		for (int i = 0; i < ehdr->e_shnum; i++) {
			const char* section_name = strtab + shdr[i].sh_name;
			
			// 指定のプレフィックスで始まるセクションを探す
			if (strncmp(section_name, SECTION_PREFIX, prefix_len) == 0) {
				// リソース名はプレフィックス以降
				std::string resource_name = section_name + prefix_len;
				if (!resource_name.empty()) {
					// リソース情報を保存
					ResourceEntry entry;
					entry.data = reinterpret_cast<const tjs_uint8*>(
						reinterpret_cast<char*>(mapped_memory) + shdr[i].sh_offset);
					entry.size = static_cast<tjs_uint>(shdr[i].sh_size);
					
					resources[resource_name] = entry;
					
					TVPAddLog(ttstr(TJS_W("Found resource: ")) + ttstr(resource_name.c_str()));
				}
			}
		}
		
		return !resources.empty();
	}

	const tjs_uint8* FindData(const ttstr &_name, tjs_uint *size=nullptr) {
		// リソース名を取得
		std::string name;
		TVPUtf16ToUtf8(name, _name.c_str());

		std::string resource_name;
		if (name.size() >= 2 && name[0] == '.' && name[1] == '/') {
			// "./" で始まる場合
			resource_name = name.substr(2);  // "./" の部分を除去
		} else {
			// 名前だけの場合
			resource_name = name.c_str();
		}
		
		// リソースマップから探す
		auto it = resources.find(resource_name);
		if (it != resources.end()) {
			// 見つかった場合はデータとサイズを返す
			if (size) *size = it->second.size;
			return it->second.data;
		}
		
		// 見つからなかった
		if (size) *size = 0;
		return nullptr;
	}

public:
	tTVPResourceStorageMedia() : RefCount(1), mapped_memory(nullptr), mapped_size(0) {
		LoadResourceFromExecutable();
	}

    ~tTVPResourceStorageMedia() {
		if (mapped_memory) {
			munmap(mapped_memory, mapped_size);
		}
	}

	void TJS_INTF_METHOD  AddRef (void) override { ++RefCount; }
	void TJS_INTF_METHOD  Release(void) override {
		if (RefCount == 1) delete this;
		else --RefCount;
	}

	//--------------------------------------------------------------

	virtual void TJS_INTF_METHOD GetName(ttstr &name) override { name = MEDIA_NAME; }
	virtual void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) override { name.ToLowerCase(); }
	virtual void TJS_INTF_METHOD NormalizePathName  (ttstr &name) override { name.ToLowerCase(); }

	virtual bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) override {
		return FindData(name) != NULL;
	}

    virtual iTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) override {
		if ((flags & TJS_BS_ACCESS_MASK) != TJS_BS_READ) return nullptr;
		tjs_uint size = 0;
		const tjs_uint8 *data = FindData(name, &size);
		if (!data) return nullptr;
		iTJSBinaryStream *stream = new tTVPMemoryStream(data, size);
		return stream;
	}

    virtual void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister * lister) override {
        if (name != "" && name != "./") return; // no list

        // リソース一覧をlisterに渡す
        for (const auto& entry : resources) {
            ttstr filename(entry.first.c_str());
            lister->Add(filename);
        }
    }

    virtual void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) override { name.Clear(); } // no local

	//--------------------------------------------------------------

	static void Load() {
		if(!Instance) {
			Instance = new Self();
			TVPRegisterStorageMedia(Instance);
		}
	}
	static void Unload() {
		if (Instance) {
			TVPUnregisterStorageMedia(Instance);
			Instance->Release();
			Instance = nullptr;
		}
	}
};

tTVPResourceStorageMedia * tTVPResourceStorageMedia::Instance = nullptr;

static tTVPAtStart AtStart(TVP_ATSTART_PRI_PREPARE, tTVPResourceStorageMedia::Load);
static tTVPAtExit  AtExit(TVP_ATEXIT_PRI_PREPARE, tTVPResourceStorageMedia::Unload);
