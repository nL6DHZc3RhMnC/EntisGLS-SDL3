#include "psb_key_resolver.h"
#include "platform/game_files.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <vector>
#include <zlib.h>

namespace entis::launcher {
namespace {
namespace fs = std::filesystem;
constexpr std::size_t kMaxDllBytes = 64u * 1024u * 1024u;
constexpr std::size_t kMaxTotalDllBytes = 256u * 1024u * 1024u;
constexpr std::size_t kMaxDlls = 128;
constexpr std::size_t kMaxCandidates = 128;
constexpr char kMarker[] = "#c#r#y#p#t#k#e#y#";
constexpr char kHelp[] = " Supply the original E-mote DLL in the game root, or set psb_key in this game's settings / entis-launcher.xml.";

std::uint16_t U16(const std::uint8_t* p) {
    return std::uint16_t(p[0]) | (std::uint16_t(p[1]) << 8);
}
std::uint32_t U32(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
bool Span(std::size_t start, std::size_t length, std::size_t size) {
    return start <= size && length <= size - start;
}
bool Decimal(const std::string& text, std::uint32_t& value) {
    if (text.empty() || text.size() > 10) return false;
    std::uint64_t n = 0;
    for (unsigned char c : text) {
        if (c < '0' || c > '9') return false;
        n = n * 10 + (c - '0');
        if (n > std::numeric_limits<std::uint32_t>::max()) return false;
    }
    value = static_cast<std::uint32_t>(n);
    return true;
}

// Standard SHA-256, independent of SDK/SDL global initialization. Source hashes
// cover the entire DLL, including overlays, not merely the marker neighborhood.
class Sha256 {
public:
    void Add(const void* data, std::size_t size) {
        const auto* p = static_cast<const std::uint8_t*>(data);
        total_ += size;
        while (size) {
            const auto n = std::min(size, block_.size() - used_);
            std::memcpy(block_.data() + used_, p, n);
            used_ += n; p += n; size -= n;
            if (used_ == block_.size()) { Transform(); used_ = 0; }
        }
    }
    std::string Finish() {
        const std::uint64_t bits = total_ * 8;
        const std::uint8_t one = 0x80, zero = 0;
        Add(&one, 1);
        while (used_ != 56) Add(&zero, 1);
        std::array<std::uint8_t, 8> length{};
        for (unsigned i = 0; i < 8; ++i) length[7 - i] = std::uint8_t(bits >> (8 * i));
        Add(length.data(), length.size());
        constexpr char hex[] = "0123456789abcdef";
        std::string out;
        for (auto word : state_) for (int shift = 28; shift >= 0; shift -= 4) out += hex[(word >> shift) & 15];
        return out;
    }
private:
    static std::uint32_t R(std::uint32_t v, unsigned n) { return (v >> n) | (v << (32 - n)); }
    void Transform() {
        constexpr std::uint32_t k[] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
        std::uint32_t w[64];
        for (unsigned i = 0; i < 16; ++i) {
            const auto* p = block_.data() + i * 4;
            w[i] = (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) | (std::uint32_t(p[2]) << 8) | p[3];
        }
        for (unsigned i = 16; i < 64; ++i) {
            const auto s0 = R(w[i-15],7) ^ R(w[i-15],18) ^ (w[i-15] >> 3);
            const auto s1 = R(w[i-2],17) ^ R(w[i-2],19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        auto a=state_[0],b=state_[1],c=state_[2],d=state_[3],e=state_[4],f=state_[5],g=state_[6],h=state_[7];
        for (unsigned i = 0; i < 64; ++i) {
            const auto s1=R(e,6)^R(e,11)^R(e,25), ch=(e&f)^(~e&g);
            const auto t1=h+s1+ch+k[i]+w[i], s0=R(a,2)^R(a,13)^R(a,22), maj=(a&b)^(a&c)^(b&c);
            h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+s0+maj;
        }
        state_[0]+=a;state_[1]+=b;state_[2]+=c;state_[3]+=d;state_[4]+=e;state_[5]+=f;state_[6]+=g;state_[7]+=h;
    }
    std::array<std::uint32_t,8> state_{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    std::array<std::uint8_t,64> block_{};
    std::uint64_t total_ = 0;
    std::size_t used_ = 0;
};

std::vector<std::uint8_t> ReadDll(const fs::path& path, std::size_t& total) {
    try {
        const auto size = io::Stat(path).size;
        if (size > kMaxDllBytes || size > kMaxTotalDllBytes - total)
            throw PsbKeyError("Cannot read game DLL within the 64 MiB/file and 256 MiB total limits." + std::string(kHelp));
        auto bytes = io::ReadFile(path, kMaxDllBytes);
        // Recheck the aggregate bound against actual data if provider metadata
        // changed between the initial stat and the bounded read.
        if (bytes.size() > kMaxTotalDllBytes - total)
            throw PsbKeyError("Game DLL total exceeds 256 MiB." + std::string(kHelp));
        total += bytes.size();
        return bytes;
    } catch (const PsbKeyError&) { throw; }
    catch (const std::exception& error) {
        throw PsbKeyError("Cannot read game DLL: " + path.filename().u8string() + ": " + error.what() + std::string(kHelp));
    }
}

void CandidatesFromPE(const std::vector<std::uint8_t>& bytes, std::set<std::uint32_t>& candidates) {
    const auto size = bytes.size();
    if (size < 64 || bytes[0] != 'M' || bytes[1] != 'Z') return;
    const auto pe = U32(bytes.data() + 60);
    if (pe < 64 || !Span(pe,24,size) || std::memcmp(bytes.data()+pe,"PE\0\0",4)) return;
    const auto sections = U16(bytes.data()+pe+6), optionalSize=U16(bytes.data()+pe+20);
    if (!(U16(bytes.data()+pe+22) & 0x2000) || sections == 0 || sections > 96 || optionalSize < 2) return;
    const std::size_t optional = static_cast<std::size_t>(pe) + 24;
    if (!Span(optional, optionalSize, size)) return;
    const auto magic = U16(bytes.data()+optional);
    if ((magic != 0x10b && magic != 0x20b) || optionalSize < (magic == 0x10b ? 96 : 112)) return;
    const auto table = optional + optionalSize;
    if (!Span(table, static_cast<std::size_t>(sections)*40, size)) return;
    const auto headersEnd = table + static_cast<std::size_t>(sections)*40;
    struct Section { std::size_t start, length; bool data; };
    std::vector<Section> ranges;
    for (unsigned i=0; i<sections; ++i) {
        const auto* s=bytes.data()+table+i*40;
        const auto length=U32(s+16), start=U32(s+20), flags=U32(s+36);
        if (length == 0) continue;
        if (start < headersEnd || !Span(start,length,size)) return;
        for (const auto& prior : ranges)
            if (start < prior.start+prior.length && prior.start < static_cast<std::size_t>(start)+length) return;
        const bool name = std::memcmp(s,".data\0\0\0",8)==0 || std::memcmp(s,".rdata\0\0",8)==0;
        ranges.push_back({start,length,name && (flags&0x40000040u)==0x40000040u && !(flags&0x20000000u)});
    }
    std::size_t markers = 0;
    for (const auto& section : ranges) {
        if (!section.data) continue;
        const auto end=section.start+section.length;
        for (auto at=section.start; Span(at,sizeof(kMarker),end); ++at) {
            if (bytes[at] != '#' || std::memcmp(bytes.data()+at,kMarker,sizeof(kMarker))) continue;
            if (++markers > kMaxCandidates) throw PsbKeyError("Too many E-mote key markers in a game DLL." + std::string(kHelp));
            if (at < section.start+2 || bytes[at-1] != 0) continue;
            auto begin=at-1;
            while (begin > section.start && bytes[begin-1]>='0' && bytes[begin-1]<='9') --begin;
            const auto length=(at-1)-begin;
            if (length == 0 || length > 10) continue;
            // The field must be a decimal string, not the numeric suffix of a
            // signed value, identifier or other printable text. Binary padding
            // immediately before the string is common in actual DLL sections.
            if (begin>section.start && bytes[begin-1]>=0x20 && bytes[begin-1]<=0x7e) continue;
            std::uint32_t key=0;
            if (Decimal(std::string(reinterpret_cast<const char*>(bytes.data()+begin),length),key)) candidates.insert(key);
            if (candidates.size()>kMaxCandidates) throw PsbKeyError("Too many E-mote key candidates." + std::string(kHelp));
        }
    }
}

// Same byte stream and v4 invariants as native/psb_header.h, but only a 56-byte
// stack copy is needed. Range checks still use the complete resource length.
bool ValidHeader(const std::uint8_t* raw, std::size_t size, std::uint32_t seed, bool encrypted) {
    std::array<std::uint8_t,56> data;
    std::memcpy(data.data(),raw,data.size());
    if (encrypted) {
        std::uint32_t x=123456789u,y=362436069u,z=521288629u,w=seed,bytes=0;
        for (std::size_t i=8;i<data.size();++i) {
            if (!bytes) { const auto t=x^(x<<11);x=y;y=z;z=w;w=w^(w>>19)^t^(t>>8);bytes=w; }
            data[i]^=static_cast<std::uint8_t>(bytes);bytes>>=8;
        }
    }
    auto checksum=adler32(1,data.data()+8,32);
    checksum=adler32(checksum,data.data()+44,12);
    if (checksum != U32(data.data()+40)) return false;
    for (std::size_t i=8;i<data.size();i+=4) if (i!=40 && U32(data.data()+i)>size) return false;
    return U32(data.data()+8)==56 && U32(data.data()+36)<size;
}

struct Discovery { std::set<std::uint32_t> candidates; std::string fingerprint; std::size_t dlls = 0; };
Discovery Discover(const fs::path& directory) {
    std::vector<fs::path> paths;
    try {
        for (const auto& entry : io::List(directory)) {
            const auto path = directory / fs::u8path(entry.name);
            auto extension = path.extension().u8string();
            for (auto& c : extension) if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
            // Following a DLL symlink could read outside the selected game.
            if (extension != ".dll" || entry.info.symbolicLink || entry.info.kind != io::FileInfo::Kind::File) continue;
            paths.push_back(path);
            if (paths.size() > kMaxDlls) throw PsbKeyError("Too many DLLs in the game directory." + std::string(kHelp));
        }
    } catch (const PsbKeyError&) { throw; }
    catch (const std::exception& error) {
        throw PsbKeyError("Cannot enumerate game DLLs: " + std::string(error.what()) + std::string(kHelp));
    }
    std::sort(paths.begin(),paths.end());
    Discovery result;
    result.dlls=paths.size();
    Sha256 snapshot;
    std::size_t total=0;
    for (const auto& path : paths) {
        const auto bytes=ReadDll(path,total);
        Sha256 hash;hash.Add(bytes.data(),bytes.size());const auto digest=hash.Finish();
        const auto name=path.filename().u8string();
        const auto length=std::to_string(name.size())+":";
        snapshot.Add(length.data(),length.size());snapshot.Add(name.data(),name.size());snapshot.Add(digest.data(),digest.size());
        CandidatesFromPE(bytes,result.candidates);
    }
    result.fingerprint=snapshot.Finish();
    return result;
}

std::string CacheContents(const std::string& fingerprint,std::uint32_t key) {
    return "entis-psb-key-cache-v1\nsha256="+fingerprint+"\nkey="+std::to_string(key)+"\n";
}
bool CacheMatches(const fs::path& path,const std::string& expected) {
    std::error_code error;
    if (!fs::is_regular_file(path,error) || error || fs::is_symlink(path,error) || error || fs::file_size(path,error)!=expected.size() || error) return false;
    std::ifstream in(path,std::ios::binary);
    if (!in) return false;
    std::string text(expected.size(),'\0');in.read(text.data(),static_cast<std::streamsize>(text.size()));
    return text==expected && in.gcount()==static_cast<std::streamsize>(text.size()) && in.peek()==std::char_traits<char>::eof() && !in.bad();
}
void WriteCache(const fs::path& path,const std::string& data) {
    fs::create_directories(path.parent_path());
    if (fs::is_symlink(path.parent_path())) throw std::runtime_error("cache directory is a symbolic link");
    static std::atomic<unsigned long long> counter{0};
    const auto suffix=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(counter++);
    const auto temp=path.parent_path()/(path.filename().u8string()+".tmp-"+suffix);
    try {
        std::ofstream out(temp,std::ios::binary|std::ios::trunc);
        out.write(data.data(),static_cast<std::streamsize>(data.size()));out.flush();
        if (!out) throw std::runtime_error("cannot write temporary cache");
        out.close();
        if (!out) throw std::runtime_error("cannot close temporary cache");
        // The temporary file is in the same directory. POSIX replaces atomically;
        // platforms refusing replacement simply keep the old (untrusted) cache.
        fs::rename(temp,path);
    } catch (...) { std::error_code ignored;fs::remove(temp,ignored);throw; }
}
} // namespace

PsbKeyResolver::PsbKeyResolver(fs::path gameDir,fs::path gameDataDir,std::optional<std::uint32_t> overrideKey)
    : gameDir_(std::move(gameDir)),gameDataDir_(std::move(gameDataDir)),overrideKey_(overrideKey) {}

std::uint32_t PsbKeyResolver::Resolve(const std::uint8_t* rawPsb,std::size_t size) {
    source_.clear();warning_.clear();
    if (!rawPsb || size<56 || std::memcmp(rawPsb,"PSB\0",4)) throw PsbKeyError("Not a complete PSB header.");
    const auto version=U16(rawPsb+4),flags=U16(rawPsb+6);
    if (version!=4 || (flags & ~1u)) throw PsbKeyError("Unsupported PSB version or encryption flags (currently PSB v4 headers only).");
    if (!flags) {
        if (!ValidHeader(rawPsb,size,0,false)) throw PsbKeyError("Unencrypted PSB header has an invalid checksum or out-of-range offsets.");
        source_="unencrypted";return 0;
    }
    if (overrideKey_) {
        if (!ValidHeader(rawPsb,size,*overrideKey_,true)) throw PsbKeyError("The configured psb_key did not validate this PSB header. Correct or remove the override; automatic fallback is disabled for an explicit value.");
        source_="explicit";return *overrideKey_;
    }
    const auto discovered=Discover(gameDir_);
    std::optional<std::uint32_t> selected;
    for (auto candidate : discovered.candidates) if (ValidHeader(rawPsb,size,candidate,true)) {
        if (selected && *selected!=candidate) throw PsbKeyError("Multiple E-mote keys validate this PSB; select psb_key explicitly.");
        selected=candidate;
    }
    if (!selected) {
        if (!discovered.dlls) throw PsbKeyError("Encrypted PSB needs a key; no game DLL is present."+std::string(kHelp));
        if (discovered.candidates.empty()) throw PsbKeyError("No supported E-mote key marker was found in the game's PE DLL data sections."+std::string(kHelp));
        throw PsbKeyError("E-mote DLL key candidates failed PSB checksum/range validation; the DLL may not match these resources, or the PSB is damaged."+std::string(kHelp));
    }
    // Cache records never authorize a key. DLL candidates AND the current PSB
    // were checked above, even when a cache file already exists.
    source_="dll";
    if (!gameDataDir_.empty()) {
        const auto path=gameDataDir_/"psb-key-cache"/(discovered.fingerprint+".cache");
        const auto contents=CacheContents(discovered.fingerprint,*selected);
        if (CacheMatches(path,contents)) source_="cache";
        else try { WriteCache(path,contents); }
        catch (const std::exception& error) { warning_="PSB key is valid, but its cache could not be written: "+std::string(error.what()); }
    }
    return *selected;
}
} // namespace entis::launcher
