// The discovery API deliberately runs before SDL/SakuraGL initialization.
#include "launcher/game_config.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace entis::launcher;
namespace {
unsigned checks = 0;
void Require(bool value, const std::string& label) {
    ++checks;
    if (!value) throw std::runtime_error(label);
}
void Reject(const std::function<void()>& action, const std::string& part) {
    try { action(); } catch (const ConfigError& e) {
        Require(std::string(e.what()).find(part) != std::string::npos,
                "Unexpected rejection: " + std::string(e.what()) + "; expected: " + part);
        return;
    }
    throw std::runtime_error("Expected config rejection: " + part);
}
void Write(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out.write(text.data(), text.size());
    if (!out) throw std::runtime_error("Cannot write fixture");
}
std::string Pe(const std::string& payload, bool plus = false) {
    const std::size_t raw = ((0x100 + payload.size() + 511) / 512) * 512;
    std::string bytes(0x200 + raw, '\0');
    auto u16 = [&](std::size_t p, unsigned x) { bytes[p] = char(x); bytes[p+1] = char(x >> 8); };
    auto u32 = [&](std::size_t p, unsigned x) { for(unsigned i=0;i<4;++i) bytes[p+i] = char(x >> (8*i)); };
    bytes[0]='M'; bytes[1]='Z'; u32(60,0x80); u32(0x80,0x4550);
    u16(0x86,1); const unsigned optSize=plus ? 240 : 224; u16(0x94,optSize);
    const unsigned opt=0x98, dirs=plus ? 112 : 96;
    u16(opt,plus ? 0x20b : 0x10b); u32(opt+dirs-4,16);
    u32(opt+dirs+16,0x1000); u32(opt+dirs+20,unsigned(raw));
    const auto section=opt+optSize;
    u32(section+8,unsigned(raw)); u32(section+12,0x1000);
    u32(section+16,unsigned(raw)); u32(section+20,0x200);
    u16(0x20e,1); u32(0x210,10); u32(0x214,0x80000020);
    u16(0x22c,1); u32(0x230,0x80000080); u32(0x234,0x80000040);
    u16(0x24e,1); u32(0x250,1041); u32(0x254,0x60);
    u32(0x260,0x1100); u32(0x264,unsigned(payload.size()));
    const std::string name="IDR_COTOMI"; u16(0x280,unsigned(name.size()));
    for(std::size_t i=0;i<name.size();++i) u16(0x282+i*2,unsigned(name[i]));
    bytes.replace(0x300,payload.size(),payload);
    return bytes;
}
}
int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: launcher_config_test <fixture-parent> [original-game-directory]\n";
        return 2;
    }
    const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = fs::absolute(argv[1]) / ("launcher-config-" + unique);
    fs::create_directories(root);
    try {
        const auto game = root / u8"game 日本語";
        fs::create_directories(game);
        Reject([&] { DiscoverGame(game); }, "No launch configuration found");
        const std::string xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<script src='story/start.csx' psb_key='0x12345678'>"
            "<save_dir path='C:\\old\\save' accept_other_dir='true'/>"
            "<archive path='$(CURRENT)\\patch.noa' id='patch' key='test&amp;key'/>"
            "<file path='$(CURRENT)'/>"
            "<archive path='./data/main.noa'/>"
            "<fonts><file name='Dialogue' path='glyphs.bmf'/><file name='Traditional' path='fonts/tc.otf'/>"
            "<filter in='Default' out='Traditional'/></fonts>"
            "<display caption='試験 &amp; Test' width='1280' height='720'/></script>";
        Write(game / "entis-launcher.xml",xml);
        Write(game / "story/start.csx","module-1");
        Write(game / "patch.noa","archive-patch");
        Write(game / "data/main.noa","archive-main");
        auto config=DiscoverGame(game);
        Require(config.entryScript=="story/start.csx","custom entry script preserved");
        Require(config.compatibilityProfile.empty(),"generic is the default");
        Require(!config.explicitGameId,"auto game identity is marked as generated");
        Require(config.psbKey && *config.psbKey==0x12345678,"explicit numeric PSB key");
        Require(config.normalizedXml.find("psb_key")==std::string::npos,"PSB key is not forwarded as an SDK setting");
        Require(!config.previousGameId.empty() && config.previousGameId!=config.gameId,"pre-0.4 key-dependent identity available for save migration");
        std::string changedKey=xml;
        changedKey.replace(changedKey.find("0x12345678"),10,"0X87654321");
        auto changed=NormalizeGameConfig(game,changedKey,"changed-key.xml");
        Require(changed.psbKey==0x87654321 && changed.gameId==config.gameId,"changing key preserves identity and updates explicit value");
        std::string noKey=xml;
        const std::string keyAttribute=" psb_key='0x12345678'";
        noKey.erase(noKey.find(keyAttribute),keyAttribute.size());
        auto automatic=NormalizeGameConfig(game,noKey,"automatic.xml");
        Require(!automatic.psbKey && automatic.gameId==config.gameId && automatic.previousGameId.empty(),"removing key preserves identity");
        Require(config.title==u8"試験 & Test","Unicode caption and XML entities preserved");
        Require(config.normalizedXml.find("local://savedata")!=std::string::npos && config.normalizedXml.find("C:")==std::string::npos,"save path isolated");
        const auto first=config.normalizedXml.find("storage://game/patch.noa");
        const auto middle=config.normalizedXml.find("path=\"storage://game\"");
        const auto last=config.normalizedXml.find("storage://game/data/main.noa");
        Require(first<middle && middle<last,"archive and file search precedence preserved");
        Require(config.normalizedXml.find("key=\"test&amp;key\"")!=std::string::npos,"archive key survives entity round trip");
        Require(config.openTypeFonts.size()==1 && config.openTypeFonts[0].family=="Traditional" && config.openTypeFonts[0].path=="fonts/tc.otf","portable OpenType font declarations extracted");
        Require(config.normalizedXml.find("tc.otf")==std::string::npos && config.normalizedXml.find("glyphs.bmf")!=std::string::npos,"SDK receives only supported bitmap font files");
        Require(config.fontAliases.size()==1 && config.fontAliases[0].alias=="Default","configured default font preserved");
        Require(config.normalizedXml.find("<filter")==std::string::npos,"font filters deferred until portable fonts load");
        auto aliases=NormalizeGameConfig(game,"<script src='main.csx'><fonts><filter in='A' out='B'/><filter in='B' out='Family'/><filter in='C' out='A'/></fonts></script>","alias-order");
        Require(aliases.fontAliases.size()==3 && aliases.fontAliases[0].alias=="B" && aliases.fontAliases[1].alias=="A" && aliases.fontAliases[2].alias=="C","font alias dependencies sorted before users");
        auto entity=NormalizeGameConfig(game,"<script src='main.csx'><display caption='&#x1F600; &#20013;'/></script>","entity");
        Require(entity.title==u8"😀 中","supplementary and decimal Unicode entities decoded");
        auto damaged=NormalizeGameConfig(game,std::string("<script src='main.csx'><display caption='A ")+char(0xa7)+" B'/></script>","damaged");
        Require(damaged.title==u8"A � B" && !damaged.warnings.empty(),"legacy invalid caption encoding repaired with warning");
        const auto moved=root/"relocated";
        fs::copy(game,moved,fs::copy_options::recursive);
        Require(DiscoverGame(moved).gameId==config.gameId,"relocation keeps game identity");
        Write(moved / "story/start.csx","module-2");
        Require(DiscoverGame(moved).gameId!=config.gameId,"different direct module separates save identity");
        Write(moved / "story/start.csx","module-1");
        Write(moved / "data/main.noa","different-archive");
        Require(DiscoverGame(moved).gameId!=config.gameId,"different archives separate save identity");
        Write(game/"cotopha.xml","<script src='wrong.csx'/>");
        Require(DiscoverGame(game).entryScript=="story/start.csx","launcher config takes precedence over legacy XML");
        Require(DiscoverGame(game,"cotopha.xml").entryScript=="wrong.csx","explicit config takes precedence");
        const auto chosen=NormalizeGameConfig(game,"<script src='main.csx' id='chosen-id' profile='study-steady-r18'/>","fixture");
        Require(chosen.gameId=="chosen-id" && chosen.explicitGameId,"explicit stable id accepted and marked");
        auto rejectXml=[&](const std::string& text,const std::string& part) {
            Reject([&] { NormalizeGameConfig(game,text,"invalid.xml"); },part);
        };
        rejectXml("<script src='main.lqs'/>","traditional Cotopha .csx only");
        rejectXml("<script src='../escape.csx'/>","Parent traversal");
        rejectXml("<script src='main.csx'><archive path='https://example.invalid/data.noa'/></script>","game-relative");
        rejectXml("<script src='main.csx'><file path='C:\\outside'/></script>","game-relative");
        rejectXml("<script src='main.csx'><file path='$(APPDATA)'/></script>","game-relative");
        rejectXml("<script src='main.csx'><module file='plugin.dll'/></script>","Windows DLL modules");
        rejectXml("<script src='main.csx'><update cmd='arbitrary.exe'/></script>","Unsupported launch configuration element");
        rejectXml("<script src='main.csx'><archive path='x.noa' download='https://invalid'/></script>","Unsupported attribute");
        rejectXml("<script src='main.csx' dynamic_env='other.xml'/>","Unsupported attribute");
        rejectXml("<script src='main.csx'><file path='.'><file path='https://invalid'/></file></script>","Unsupported nested");
        rejectXml("<script src='main.csx' src='other.csx'/>","Duplicate XML attribute");
        rejectXml("<script src='main.csx'><fonts></script>","Mismatched XML closing");
        rejectXml("<script src='main.csx'/><script src='other.csx'/>","exactly one XML root");
        rejectXml("<!DOCTYPE script [<!ENTITY xx 'bad'>]><script src='main.csx'/>","DTD");
        rejectXml("<script src='main.csx' psb_key='4294967296'/>","32-bit unsigned");
        rejectXml("<script src='main.csx' psb_key='-1'/>","32-bit unsigned");
        rejectXml("<script src='main.csx' psb_key='+1'/>","32-bit unsigned");
        rejectXml("<script src='main.csx' psb_key=' 1'/>","32-bit unsigned");
        rejectXml("<script src='main.csx' id='../outside'/>","Invalid game id");
        rejectXml("<script src='main.csx' id='CON'/>","Invalid game id");
        rejectXml("<script src='main.csx'><fonts><filter in='A' out='B'/><filter in='B' out='A'/></fonts></script>","Cyclic font filters");
        rejectXml("<script src='main.csx'><fonts><filter in='A' out='B'/><filter in='A' out='C'/></fonts></script>","Duplicate font filter");
        rejectXml("<script src='&#46;&#46;/escape.csx'/>","Parent traversal");
        rejectXml("<script src='main.csx'><display caption='&external;'/></script>","Unsupported XML entity");
        Write(root/"outside.csx","outside");
        fs::create_symlink(root/"outside.csx",game/"link.csx");
        rejectXml("<script src='link.csx'/>","escapes the selected game directory");
        const auto peGame=root/"pe32";
        Write(peGame/"engine.exe",Pe("<script src='custom.csx'><file path='.'/></script>"));
        auto fromPe=DiscoverGame(peGame);
        Require(fromPe.entryScript=="custom.csx" && fromPe.configSource=="engine.exe:IDR_COTOMI","PE32 configuration extraction without executing EXE");
        Write(peGame/"second.exe",Pe(std::string("<script src='another.csx'/>")+char(0),true));
        Reject([&] { DiscoverGame(peGame); },"Multiple game executables");
        Require(DiscoverGame(peGame,"second.exe").entryScript=="another.csx","PE32+ explicit extraction");
        auto broken=Pe("<script src='custom.csx'/>"); broken.resize(100);
        Write(peGame/"broken.exe",broken);
        Reject([&] { DiscoverGame(peGame,"broken.exe"); },"Truncated PE");
        if(argc==3) {
            auto real=DiscoverGame(fs::u8path(argv[2]));
            Require(real.entryScript=="script.csx","real compressed IDR_COTOMI entry");
            Require(real.compatibilityProfile.empty(),"real EXE does not implicitly activate game-specific compatibility");
            Require(real.normalizedXml.find("patch2.noa")<real.normalizedXml.find("patch1.noa") && real.normalizedXml.find("patch1.noa")<real.normalizedXml.find("script.noa"),"real game's patch precedence preserved");
            Require(real.normalizedXml.find("movie2.noa")!=std::string::npos && real.normalizedXml.find("bg2.noa")!=std::string::npos,"original optional resources retained");
            std::cout << "Real config: " << real.configSource << " id=" << real.gameId << " warnings=" << real.warnings.size() << '\n';
        }
        fs::remove_all(root);
        std::cout << "Launcher configuration PASS: " << checks << " checks; no SDL or SDK initialization\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Launcher configuration FAIL: " << error.what() << " (fixtures " << root << ")\n";
        return 1;
    }
}
