// Synthetic registered fonts only: no installed fonts or game data are needed.
#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_setup.h"
#include "platform/sdl/system.h"
#include <sakuragl/sgl2d/sgl_font.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace SakuraGL;
constexpr const wchar_t* latinName=L"Synthetic One";
constexpr const wchar_t* japaneseName=L"Synthetic Two";

void Require(bool value, const char* detail) {
    if (!value) throw std::runtime_error(detail);
}

class CoverageFont final : public SGLFontObject {
public:
    explicit CoverageFont(std::uint32_t glyph) : glyph_(glyph) {}
    SGLFontObject* NewFont(const SGLFontStyle& style) override {
        if (!style.nSize) return nullptr;
        return new CoverageFont(glyph_);
    }
    SGLError SetStyle(const SGLFontStyle& style) override {
        return style.nSize ? sglErrSuccess : sglErrInvalidParam;
    }
    SGLError GetMetrics(std::uint8_t* raster, size_t bytes,
                        SGLFontMetrics& metrics, std::uint32_t glyph) override {
        if (glyph!=glyph_) return sglErrFailed;
        metrics={};
        metrics.nWidth=metrics.nHeight=metrics.nAscent=1;
        metrics.rctExterior.w=metrics.rctExterior.h=1;
        if (raster) {
            if (!bytes) return sglErrInvalidParam;
            raster[0]=255;
        }
        return sglErrSuccess;
    }
private:
    std::uint32_t glyph_;
};

INT64 Invoke(ECSContext& context, ECSSetup& setup,
             std::initializer_list<ECSObject*> parameters) {
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&setup));
    for (auto* argument : parameters) args.Add(argument);
    int index=-1;
    Require(!setup.GetFunction(context,index,L"GetFontList"), "GetFontList must resolve");
    Require(!setup.CallFunction(context,index,args), "GetFontList must return without a VM error");
    std::unique_ptr<ECSObject> result(context.PopObject());
    INT64 count=-1;
    Require(result && !result->OperateInteger(count), "GetFontList returns an integer count");
    return count;
}

std::vector<std::wstring> Names(const ECSArray& array) {
    std::vector<std::wstring> names;
    for (unsigned i=0;i<array.m_varArray.GetSize();++i) {
        const auto* text=ESLTypeCast<ECSString>(array.m_varArray.GetAt(i));
        Require(text && text->m_varStr.CharPtr(), "enumerated font names are strings");
        names.emplace_back(text->m_varStr.CharPtr());
    }
    return names;
}
bool Contains(const ECSArray& array, const wchar_t* name) {
    const auto names=Names(array);
    return std::find(names.begin(),names.end(),name)!=names.end();
}
void Reject(ECSContext& context, ECSSetup& setup,
            std::initializer_list<ECSObject*> parameters) {
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&setup));
    for (auto* argument : parameters) args.Add(argument);
    int index=-1;
    Require(!setup.GetFunction(context,index,L"GetFontList"), "argument-check method resolves");
    Require(setup.CallFunction(context,index,args)!=eslErrSuccess,
            "invalid GetFontList arguments must raise a VM error");
}

void Run() {
    struct Runtime {
        Runtime() { ECotophaScript::Initialize(0); }
        ~Runtime() { ECotophaScript::Release(); }
    } runtime;
    struct Registry {
        Registry() { SGLFont::FinalizeRemapFontTable(); }
        ~Registry() { SGLFont::FinalizeRemapFontTable(); }
    } registry;
    ECSContext context;
    ECSSetup setup;
    ECSArray empty;
    Require(Invoke(context,setup,{new ECSReference(&empty)})==0 && empty.m_varArray.GetSize()==0,
            "an empty registered-font list produces an empty result");

    // Each fixture supports only one representative glyph. The filter must
    // inspect that coverage instead of inferring a charset from its name.
    SGLFont::RegisterStockFont(latinName,new CoverageFont(L'A'));
    SGLFont::RegisterStockFont(japaneseName,new CoverageFont(0x3042));
    ECSArray all;
    all.m_varArray.Add(new ECSString(L"existing entry"));
    Require(Invoke(context,setup,{new ECSReference(&all),new ECSInteger(0)})==3 &&
            Names(all).front()==L"existing entry" && Contains(all,latinName) && Contains(all,japaneseName),
            "flags 0 appends both fonts and returns the total array length");
    Require(Invoke(context,setup,{new ECSReference(&all)})==5 && Names(all).front()==L"existing entry",
            "omitted flags default to all and repeated calls append rather than clear");

    ECSArray defaults;
    Require(Invoke(context,setup,{new ECSReference(&defaults),new ECSInteger(1)})==2 &&
            Contains(defaults,latinName) && Contains(defaults,japaneseName),
            "default charset flag includes all registered fonts");
    ECSArray japanese;
    Require(Invoke(context,setup,{new ECSReference(&japanese),new ECSInteger(4)})==1 &&
            Contains(japanese,japaneseName) && !Contains(japanese,latinName),
            "Japanese filter selects a font supporting U+3042");
    ECSArray latin;
    Require(Invoke(context,setup,{new ECSReference(&latin),new ECSInteger(2)})==1 &&
            Contains(latin,latinName) && !Contains(latin,japaneseName),
            "Latin filter selects a font supporting A");
    ECSArray either;
    Require(Invoke(context,setup,{new ECSReference(&either),new ECSInteger(6)})==2 &&
            Contains(either,latinName) && Contains(either,japaneseName),
            "combined coverage flags include either supported character set");
    ECSArray unknown;
    Require(Invoke(context,setup,{new ECSReference(&unknown),new ECSInteger(0x80)})==0 &&
            unknown.m_varArray.GetSize()==0, "unknown-only flags do not fabricate a match");
    unknown.m_varArray.Add(new ECSString(L"preserved"));
    Require(Invoke(context,setup,{new ECSReference(&unknown),new ECSInteger(8)})==1 &&
            Names(unknown)==std::vector<std::wstring>{L"preserved"},
            "unsupported symbol charset leaves existing entries unchanged and returns their count");
    Reject(context,setup,{new ECSString(L"not an Array")});
    Reject(context,setup,{});
}
}

int main() {
    if (!SDL_Init(0)) return 2;
    bool initialized=false;
    bool createdRoot=false;
    std::filesystem::path root;
    int result=0;
    try {
        namespace fs=std::filesystem;
        const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        root=fs::temp_directory_path()/("entis-setup-fonts-test-"+unique);
        createdRoot=fs::create_directory(root);
        Require(createdRoot,"cannot create isolated fixture directory");
        for (const char* name : {"assets","storage","local","game"}) fs::create_directory(root/name);
        study::platform::sdl::SystemPaths paths;
        paths.assetsRoot=(root/"assets").string();
        paths.storageRoot=(root/"storage").string();
        paths.localRoot=(root/"local").string();
        paths.gameRoot=(root/"game").string();
        Require(study::platform::sdl::ConfigureSystemPaths(paths),"isolated fixture paths rejected");
        SakuraGL::Initialize();
        initialized=true;
        Run();
        std::puts("Legacy Setup font list PASS: append/count, empty registry, coverage filters, unsupported flags and invalid arguments");
    } catch (const std::exception& error) {
        std::fprintf(stderr,"Legacy Setup font list FAIL: %s\n",error.what());
        result=1;
    }
    if (initialized) SakuraGL::Finalize();
    SDL_Quit();
    if (createdRoot) {
        std::error_code ignored;
        std::filesystem::remove_all(root,ignored);
    }
    return result;
}
