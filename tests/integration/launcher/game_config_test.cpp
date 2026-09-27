// The discovery API deliberately runs before SDL/SakuraGL initialization.
#include "launcher/game_config.h"
#include "../../fixtures/game_file_backend_fixture.h"
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
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
    u16(0x84,plus ? 0x8664 : 0x14c);
    u16(0x86,1); const unsigned optSize=plus ? 240 : 224; u16(0x94,optSize);
    const unsigned opt=0x98, dirs=plus ? 112 : 96;
    u16(opt,plus ? 0x20b : 0x10b); u32(opt+dirs-4,16);
    u32(opt+32,0x1000); u32(opt+36,0x200);
    u32(opt+56,0x1000+unsigned(raw)); u32(opt+60,0x200);
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
std::string PeWithoutConfig(const std::string& sectionPayload = {}) {
    auto bytes = Pe(sectionPayload);
    // A valid wrapper with file-backed content but no resource directory.
    bytes.replace(0x98+96+16, 8, 8, '\0');
    return bytes;
}
void Put32(std::string& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned i=0;i<4;++i) bytes[offset+i]=char(value>>(8*i));
}
void Put64(std::string& bytes, std::size_t offset, std::uint64_t value) {
    for (unsigned i=0;i<8;++i) bytes[offset+i]=char(value>>(8*i));
}
std::string CsxHeader() {
    // Enough of an independently authored image to prove format discovery;
    // discovery must not execute the image or initialize the SDK VM.
    std::string bytes(64,'\0');
    bytes.replace(0,8,"Entis\x1a\0\0",8);
    Put32(bytes,8,0xffffffff);
    bytes.replace(16,18,"Cotopha Image file",18);
    return bytes;
}
class MemoryOutput final : public SSystem::SOutputStream {
public:
    std::string bytes;
    size_t Write(const void* data, size_t count) override {
        bytes.append(static_cast<const char*>(data),count);
        return count;
    }
};
std::string Erisan(const std::string& plain) {
    // Use the SDK encoder on the authored fixture, never copy compressed game
    // resources or mirror the production decoder in the test.
    MemoryOutput output;
    ERISA::SGLEncodeBitStream bits(0x1000);
    bits.AttachOutputStream(&output);
    ERISA::SGLERISANEncodeContext encoder(&bits);
    encoder.PrepareToEncodeERISANCode();
    if (encoder.Write(plain.data(),plain.size())!=plain.size())
        throw std::runtime_error("Cannot encode ERISAN fixture");
    encoder.EncodeERISANCodeEOF();
    if (encoder.FinishEncoding()!=0 || output.bytes.empty())
        throw std::runtime_error("Cannot finalize ERISAN fixture");
    return output.bytes;
}
struct ArchiveEntry {
    std::string name;
    std::string data;
    bool compressed=false;
};
std::string Noa(const std::vector<ArchiveEntry>& entries) {
    std::size_t indexSize=4;
    for (const auto& entry:entries) indexSize+=40+entry.name.size()+1;
    std::string bytes(80+indexSize,'\0');
    bytes.replace(0,8,"Entis\x1a\0\0",8);
    Put32(bytes,8,0x02000400);
    bytes.replace(64,8,"DirEntry",8);
    Put64(bytes,72,indexSize);
    Put32(bytes,80,std::uint32_t(entries.size()));
    std::size_t at=84;
    for (const auto& entry:entries) {
        Put64(bytes,at,entry.data.size());
        Put32(bytes,at+8,0x01000000); // UTF-8 name, ordinary file entry.
        Put32(bytes,at+12,entry.compressed ? 0x80000010 : 0);
        Put64(bytes,at+16,bytes.size()-64);
        Put32(bytes,at+36,std::uint32_t(entry.name.size()+1));
        bytes.replace(at+40,entry.name.size(),entry.name);
        at+=40+entry.name.size()+1;
        const auto stored=entry.compressed ? Erisan(entry.data) : entry.data;
        const auto payload=bytes.size();
        bytes.resize(payload+16+stored.size(),'\0');
        bytes.replace(payload,8,"filedata",8);
        Put64(bytes,payload+8,stored.size());
        bytes.replace(payload+16,stored.size(),stored);
    }
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
        const auto unchangedIdentity = NormalizeGameConfig(game, "<script src='main.csx'/>", "save-policy-regression");
        Require(unchangedIdentity.gameId == "game-6d019a001cd781cf", "changing save placement preserves pre-provider game identity");
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
        {
            const auto virtualRoot = root / "provider-game";
            auto backend = std::make_shared<MappedGameTestBackend>(game);
            GameTestMount mount(virtualRoot, backend);
            Require(!fs::exists(virtualRoot), "virtual game requires no copied resource directory");
            const auto linked = DiscoverGame(virtualRoot);
            Require(linked.gameId == config.gameId && linked.previousGameId == config.previousGameId,
                    "provider-backed game preserves existing settings and save identity");
            Require(linked.normalizedXml == config.normalizedXml, "provider-backed configuration is unchanged");
            Require(backend->opens >= 4 && backend->stats > 0, "discovery reads configuration and resource identity through provider");
            Reject([&] { DiscoverGame(virtualRoot, "../outside.xml"); }, "escapes");
            bool escaped = false;
            try { entis::io::Open(virtualRoot / "../outside.xml", "rb"); }
            catch (const std::exception&) { escaped = true; }
            Require(escaped, "provider traversal cannot fall back to host IO");
            entis::io::CreateDirectory(virtualRoot / "writable");
            auto* stream = entis::io::Open(virtualRoot / "writable/from.txt", "wb");
            const auto writtenCount = std::fwrite("direct", 1, 6, stream);
            const auto closed = std::fclose(stream);
            Require(writtenCount == 6 && closed == 0, "provider writes directly to selected directory");
            entis::io::Rename(virtualRoot / "writable/from.txt", virtualRoot / "writable/to.txt");
            const auto written = entis::io::ReadFile(virtualRoot / "writable/to.txt", 6);
            Require(std::string(written.begin(), written.end()) == "direct" && fs::exists(game / "writable/to.txt"), "provider rename and bounded read use original directory");
            bool limited = false;
            try { entis::io::ReadFile(virtualRoot / "writable/to.txt", 5); }
            catch (const std::exception&) { limited = true; }
            Require(limited, "provider read respects resource limit");
            entis::io::Remove(virtualRoot / "writable/to.txt", false);
            entis::io::Remove(virtualRoot / "writable", true);
            Require(!fs::exists(game / "writable"), "provider removal updates selected directory");
        }
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
        Require(config.normalizedXml.find("storage://game/savedata")!=std::string::npos && config.normalizedXml.find("C:")==std::string::npos,"save path is the selected game directory");
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
        rejectXml("<script><display width='1280' height='720'/></script>","No Cotopha .csx entry script is declared in invalid.xml");
        rejectXml("<script src=''/>","missing or empty <script src>");
        rejectXml("<cotopha/>","missing or empty <cotopha src>");
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
        const auto nativeGame=root/"native-environment";
        Write(nativeGame/"engine.exe",Pe("<script><archive path='data.noa'/><display width='1280' height='720'/></script>"));
        Reject([&] { DiscoverGame(nativeGame); },"No Cotopha .csx entry script is declared in engine.exe:IDR_COTOMI");
        Write(peGame/"second.exe",Pe(std::string("<script src='another.csx'/>")+char(0),true));
        Reject([&] { DiscoverGame(peGame); },"Multiple game executables");
        Require(DiscoverGame(peGame,"second.exe").entryScript=="another.csx","PE32+ explicit extraction");
        const auto embeddedGame=root/"embedded-pe";
        const auto wrapper=PeWithoutConfig("wrapper data");
        const auto embeddedXml="<script src='embedded.csx'><file path='.'/></script>";
        const auto nested=Pe(embeddedXml,true);
        // An unaligned offset after a real section exercises container-relative
        // PE addressing; no production game or fixed wrapper offset is used.
        Write(embeddedGame/"wrapped.exe",wrapper+std::string(137,'x')+nested);
        auto fromEmbedded=DiscoverGame(embeddedGame);
        Require(fromEmbedded.entryScript=="embedded.csx", "configuration discovered in appended PE32+ image");
        Require(fromEmbedded.configSource.find("wrapped.exe")!=std::string::npos &&
                fromEmbedded.configSource.find("IDR_COTOMI")!=std::string::npos,
                "embedded configuration identifies its source executable");
        Require(DiscoverGame(embeddedGame,"wrapped.exe").normalizedXml==fromEmbedded.normalizedXml,
                "explicit executable selection also extracts embedded configuration");
        {
            const auto virtualRoot=root/"provider-embedded";
            auto backend=std::make_shared<MappedGameTestBackend>(embeddedGame);
            GameTestMount mount(virtualRoot,backend);
            const auto linked=DiscoverGame(virtualRoot);
            Require(linked.entryScript=="embedded.csx" && linked.normalizedXml==fromEmbedded.normalizedXml,
                    "embedded extraction works through selected-directory provider");
            Require(backend->opens>0 && backend->lists>0, "embedded discovery uses provider IO");
        }
        Write(embeddedGame/"wrapped.exe",Pe("<script src='outer.csx'/>")+nested);
        Require(DiscoverGame(embeddedGame).entryScript=="outer.csx",
                "outer executable configuration takes precedence over embedded images");
        Write(embeddedGame/"wrapped.exe",wrapper+nested+std::string(29,'x')+nested);
        Require(DiscoverGame(embeddedGame).entryScript=="embedded.csx",
                "identical embedded configurations are not ambiguous");
        Write(embeddedGame/"wrapped.exe",wrapper+nested+Pe("<script src='different.csx'/>"));
        Reject([&] { DiscoverGame(embeddedGame); },"multiple different IDR_COTOMI");
        auto malformedCandidate=Pe("<script src='bad.csx'/>");
        Put32(malformedCandidate,0x98+224+16,0xffffffff); // Raw section exceeds container.
        Write(embeddedGame/"wrapped.exe",wrapper+malformedCandidate+std::string(51,'x')+nested);
        Require(DiscoverGame(embeddedGame).entryScript=="embedded.csx",
                "malformed embedded PE decoy does not hide a later valid image");
        Write(embeddedGame/"wrapped.exe",PeWithoutConfig(nested));
        Reject([&] { DiscoverGame(embeddedGame); },"No launch configuration found");
        Write(embeddedGame/"wrapped.exe",std::string("not a Windows executable")+nested);
        Reject([&] { DiscoverGame(embeddedGame); },"No launch configuration found");
        Write(embeddedGame/"wrapped.exe",wrapper+nested);
        Write(embeddedGame/"entis-launcher.xml","<script src='override.csx'/>");
        Require(DiscoverGame(embeddedGame).entryScript=="override.csx",
                "explicit launcher XML takes precedence over wrapper discovery");
        auto broken=Pe("<script src='custom.csx'/>"); broken.resize(100);
        Write(peGame/"broken.exe",broken);
        Reject([&] { DiscoverGame(peGame,"broken.exe"); },"Truncated PE");
        const auto fallbackGame=root/"archive-fallback";
        const auto scriptArchive=Noa({{"script.csx",CsxHeader()}});
        Write(fallbackGame/"script.noa",scriptArchive);
        const auto fallback=DiscoverGame(fallbackGame);
        Require(fallback.entryScript=="script.csx" && fallback.configSource=="script.noa:script.csx (fallback)",
                "verified archive entry supplies configuration when original config is absent");
        Require(fallback.compatibilityProfile.empty() && !fallback.psbKey,
                "archive fallback is generic and has no built-in game profile or key");
        Require(!fallback.warnings.empty(), "inferred launch configuration reports its limitations");
        Require(!fs::exists(fallbackGame/"cotopha.xml") && !fs::exists(fallbackGame/"entis-launcher.xml"),
                "fallback does not write inferred configuration into game resources");
        {
            const auto virtualRoot=root/"provider-fallback";
            auto backend=std::make_shared<MappedGameTestBackend>(fallbackGame);
            GameTestMount mount(virtualRoot,backend);
            const auto linked=DiscoverGame(virtualRoot);
            Require(linked.entryScript=="script.csx" && linked.normalizedXml==fallback.normalizedXml &&
                    linked.gameId==fallback.gameId, "archive fallback works through selected-directory provider");
            Require(backend->opens>0 && backend->lists>0, "fallback probes resources through provider IO");
        }
        const auto compressedGame=root/"compressed-archive-fallback";
        Write(compressedGame/"script.noa",Noa({{"script.csx",CsxHeader(),true}}));
        const auto compressed=DiscoverGame(compressedGame);
        Require(compressed.entryScript=="script.csx" && compressed.normalizedXml==fallback.normalizedXml,
                "ERISAN-compressed CSX header is decoded during archive fallback");
        Write(compressedGame/"script.noa",Noa({{"script.csx",std::string(64,'x'),true}}));
        Reject([&] { DiscoverGame(compressedGame); },"Cotopha Image file");
        Write(fallbackGame/"patch2.noa",Noa({{"patch.dat","two"}}));
        Write(fallbackGame/"patch10.noa",Noa({{"patch.dat","ten"}}));
        Write(fallbackGame/"bg.noa",Noa({{"background.eri","fixture"}}));
        Write(fallbackGame/"system.noa",Noa({{"msgfont.bmf","fixture font"},{"msgfont_v.bmf","fixture vertical font"}}));
        const auto inferred=DiscoverGame(fallbackGame);
        const auto patch10=inferred.normalizedXml.find("storage://game/patch10.noa");
        const auto patch2=inferred.normalizedXml.find("storage://game/patch2.noa");
        const auto base=inferred.normalizedXml.find("storage://game/bg.noa");
        const auto script=inferred.normalizedXml.find("storage://game/script.noa");
        const auto loose=inferred.normalizedXml.find("<file path=\"storage://game\"");
        Require(patch10!=std::string::npos && patch10<patch2 && patch2<script && script<base && base<loose,
                "inferred mounts place higher numbered patches first, then script and base archives, then loose files");
        Require(inferred.normalizedXml.find("msgfont.bmf")!=std::string::npos &&
                inferred.normalizedXml.find("msgfont_v.bmf")!=std::string::npos,
                "fallback registers conventional bitmap fonts only when present in resources");
        Write(fallbackGame/"engine.exe",PeWithoutConfig("no configuration"));
        Require(DiscoverGame(fallbackGame).entryScript=="script.csx",
                "archive fallback is available when executable exposes no configuration");
        Reject([&] { DiscoverGame(fallbackGame,"engine.exe"); },"Executable has no RCDATA/IDR_COTOMI");
        Write(fallbackGame/"engine.exe","damaged executable");
        Require(DiscoverGame(fallbackGame).entryScript=="script.csx",
                "automatic discovery can fall back after executable extraction fails");
        Write(fallbackGame/"engine.exe",Pe("<script src='declared.csx'/>") );
        Require(DiscoverGame(fallbackGame).entryScript=="declared.csx",
                "real executable configuration takes precedence over archive fallback");
        Write(fallbackGame/"engine.exe",Pe("<script/>"));
        Reject([&] { DiscoverGame(fallbackGame); },"No Cotopha .csx entry script is declared");
        fs::remove(fallbackGame/"engine.exe");
        Write(fallbackGame/"cotopha.xml","<script src='manual.csx'/>");
        Require(DiscoverGame(fallbackGame).entryScript=="manual.csx",
                "legacy XML configuration takes precedence over archive fallback");
        Write(fallbackGame/"cotopha.xml","<script src='../escape.csx'/>");
        Reject([&] { DiscoverGame(fallbackGame); },"Parent traversal");
        fs::remove(fallbackGame/"cotopha.xml");
        Write(fallbackGame/"script.noa",Noa({{"main.srcxml","<xscript/>"}}));
        Reject([&] { DiscoverGame(fallbackGame); },"No launch configuration found");
        Write(fallbackGame/"script.noa",Noa({{"script.csx",std::string(64,'x')}}));
        Reject([&] { DiscoverGame(fallbackGame); },"Cotopha Image file");
        Write(fallbackGame/"script.noa",Noa({{"script.csx","short"}}));
        Reject([&] { DiscoverGame(fallbackGame); },"size is outside");
        Write(fallbackGame/"script.noa",scriptArchive.substr(0,90));
        Reject([&] { DiscoverGame(fallbackGame); },"Archive fallback");
        Write(fallbackGame/"script.noa",scriptArchive);
        Write(fallbackGame/"patch10.noa",Noa({{"script.csx",std::string(64,'x')}}));
        Reject([&] { DiscoverGame(fallbackGame); },"Cotopha Image file");
        Write(fallbackGame/"patch10.noa",Noa({{"patch.dat","ten"}}));
        const auto upperGame=root/"uppercase-archive";
        Write(upperGame/"SCRIPT.NOA",Noa({{"SCRIPT.CSX",CsxHeader()}}));
        Require(DiscoverGame(upperGame).entryScript=="SCRIPT.CSX", "fallback accepts actual case of archive and entry names");
        auto invalidOffset=scriptArchive;
        Put64(invalidOffset,84+16,0xffffffffffffffffULL);
        Write(fallbackGame/"script.noa",invalidOffset);
        Reject([&] { DiscoverGame(fallbackGame); },"Archive fallback");
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
