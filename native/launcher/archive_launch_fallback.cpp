#include "launcher/archive_launch_fallback.h"
#include "io/game_files.h"

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>

namespace entis::launcher {
namespace {
namespace fs = std::filesystem;
constexpr std::size_t maxArchives = 256;
constexpr std::size_t maxIndex = 16 * 1024 * 1024;
constexpr std::size_t maxTotalIndices = 64 * 1024 * 1024;
constexpr std::uint64_t maxScript = 256 * 1024 * 1024;
constexpr std::uint64_t maxHeaderInput = 1024 * 1024;
constexpr char signature[] = "Entis\x1a\0\0";

std::string Lower(std::string text) {
    for (char& c : text) if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    return text;
}
std::uint32_t U32(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
std::uint64_t U64(const std::uint8_t* p) {
    return std::uint64_t(U32(p)) | (std::uint64_t(U32(p + 4)) << 32);
}
bool Span(std::uint64_t offset, std::uint64_t length, std::uint64_t size) {
    return offset <= size && length <= size - offset;
}
[[noreturn]] void Invalid(const fs::path& path, const std::string& reason) {
    throw ConfigError("Archive fallback: " + path.filename().u8string() + ": " + reason);
}
std::string Escape(const std::string& value) {
    std::string result;
    for (const char c : value) {
        if (c == '&') result += "&amp;";
        else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '"') result += "&quot;";
        else result += c;
    }
    return result;
}

class ArchiveInput {
public:
    explicit ArchiveInput(const fs::path& path)
        : path_(path), size_(io::Stat(path).size), file_(io::Open(path, "rb"), &std::fclose) {}
    std::uint64_t Size() const { return size_; }
    std::FILE* Stream() const { return file_.get(); }
    void Read(std::uint64_t offset, void* output, std::size_t count) {
        if (!Span(offset, count, size_) || offset > std::uint64_t(std::numeric_limits<std::int64_t>::max()))
            Invalid(path_, "truncated NOA data or invalid offset");
#if defined(_WIN32)
        const auto seek = _fseeki64(file_.get(), static_cast<std::int64_t>(offset), SEEK_SET);
#else
        if (offset > std::uint64_t(std::numeric_limits<off_t>::max())) Invalid(path_, "NOA offset exceeds platform limits");
        const auto seek = fseeko(file_.get(), static_cast<off_t>(offset), SEEK_SET);
#endif
        if (seek || std::fread(output, 1, count, file_.get()) != count)
            Invalid(path_, "cannot read NOA data");
    }
private:
    fs::path path_;
    std::uint64_t size_;
    std::unique_ptr<std::FILE, decltype(&std::fclose)> file_;
};

struct Entry {
    std::string name;
    std::uint64_t size;
    std::uint32_t encoding;
    std::uint64_t offset;
};
struct Index {
    std::optional<Entry> script;
    std::optional<Entry> font;
    std::optional<Entry> verticalFont;
};

// Only the root directory and three standard ASCII names are relevant here.
// Avoid SDK archive initialization and avoid decoding unrelated CP932 names.
Index ReadIndex(const fs::path& path, std::size_t& totalIndices) {
    ArchiveInput input(path);
    std::array<std::uint8_t, 80> header{};
    input.Read(0, header.data(), header.size());
    if (std::memcmp(header.data(), signature, 8) || U32(header.data() + 8) != 0x02000400 ||
        std::memcmp(header.data() + 64, "DirEntry", 8))
        Invalid(path, "invalid NOA header or root DirEntry");
    const auto size = U64(header.data() + 72);
    if (size < 4 || size > maxIndex || size > maxTotalIndices - totalIndices)
        Invalid(path, "NOA directory exceeds discovery limits (16 MiB each, 64 MiB total)");
    if (!Span(80, size, input.Size())) Invalid(path, "truncated NOA directory");
    totalIndices += static_cast<std::size_t>(size);
    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    input.Read(80, data.data(), data.size());
    const auto count = U32(data.data());
    if (count > (size - 4) / 41) Invalid(path, "invalid NOA directory entry count");
    Index result;
    std::size_t at = 4;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (!Span(at, 36, data.size())) Invalid(path, "truncated NOA directory entry");
        const auto* p = data.data() + at;
        Entry entry{{}, U64(p), U32(p + 12), U64(p + 16)};
        const auto attributes = U32(p + 8), extra = U32(p + 32);
        at += 36;
        if (!Span(at, extra, data.size())) Invalid(path, "invalid NOA extra-information length");
        at += extra;
        if (!Span(at, 4, data.size())) Invalid(path, "truncated NOA filename length");
        const auto length = U32(data.data() + at);
        at += 4;
        if (!length || !Span(at, length, data.size()) || data[at + length - 1] != 0 ||
            std::memchr(data.data() + at, 0, length - 1))
            Invalid(path, "invalid NOA filename");
        // Retain only selected metadata, not a copy of every filename/index.
        entry.name.assign(reinterpret_cast<const char*>(data.data() + at), length - 1);
        at += length;
        if (attributes & 0x70) continue; // directories and directory control entries
        if (entry.offset < 16 + size || !Span(64, entry.offset, input.Size()) ||
            !Span(64 + entry.offset, 16, input.Size()))
            Invalid(path, "invalid NOA file offset");
        const auto name = Lower(entry.name);
        auto* selected = name == "script.csx" ? &result.script : name == "msgfont.bmf" ? &result.font :
                         name == "msgfont_v.bmf" ? &result.verticalFont : nullptr;
        if (!selected) continue;
        if (*selected) Invalid(path, "duplicate standard resource name: " + name);
        *selected = std::move(entry);
    }
    if (at != data.size()) Invalid(path, "unexpected NOA directory trailer");
    return result;
}

// ERISAN can decode a small prefix without materializing the whole script.
// Both consumed compressed bytes and requested output are bounded.
class LimitedInput final : public SSystem::SInputStream {
public:
    LimitedInput(std::FILE* file, std::uint64_t bytes) : file_(file), remaining_(bytes) {}
    size_t Read(void* output, size_t count) override {
        count = static_cast<std::size_t>(std::min<std::uint64_t>(count, remaining_));
        const auto got = std::fread(output, 1, count, file_);
        remaining_ -= got;
        return got;
    }
private:
    std::FILE* file_;
    std::uint64_t remaining_;
};

void ValidateScript(const fs::path& path, const Entry& entry) {
    if (entry.size < 64 || entry.size > maxScript)
        Invalid(path, "script.csx size is outside the supported 64-byte to 256-MiB range");
    ArchiveInput input(path);
    std::array<std::uint8_t, 16> chunk{};
    input.Read(64 + entry.offset, chunk.data(), chunk.size());
    const auto stored = U64(chunk.data() + 8);
    const auto start = 80 + entry.offset;
    if (std::memcmp(chunk.data(), "filedata", 8) || !Span(start, stored, input.Size()))
        Invalid(path, "truncated or invalid script.csx filedata chunk");
    std::array<std::uint8_t, 64> header{};
    if (entry.encoding == 0) {
        if (stored != entry.size) Invalid(path, "raw script.csx length does not match the NOA index");
        input.Read(start, header.data(), header.size());
    } else if (entry.encoding == 0x80000010) {
        // The previous chunk-header read leaves the stream at its payload.
        LimitedInput stream(input.Stream(), std::min(stored, maxHeaderInput));
        ERISA::SGLDecodeBitStream bits(0x4000);
        bits.AttachInputStream(&stream);
        ERISA::SGLERISANDecodeContext decoder(&bits);
        decoder.PrepareToDecodeERISANCode();
        if (decoder.Read(header.data(), header.size()) != header.size())
            Invalid(path, "cannot decode the script.csx Cotopha Image file header within the read limit");
    } else {
        Invalid(path, "unsupported script.csx archive encoding; supply an explicit launch configuration with any required archive key");
    }
    constexpr char description[] = "Cotopha Image file";
    if (std::memcmp(header.data(), signature, 8) || U32(header.data() + 8) != 0xffffffff ||
        std::memcmp(header.data() + 16, description, sizeof(description)))
        Invalid(path, "script.csx is not a Cotopha Image file");
}

std::optional<std::string> PatchNumber(const std::string& filename) {
    const auto stem = Lower(fs::u8path(filename).stem().u8string());
    if (stem.rfind("patch", 0) != 0) return {};
    auto number = stem.substr(5);
    if (number.find_first_not_of("0123456789") != std::string::npos) return {};
    const auto first = number.find_first_not_of('0');
    return first == std::string::npos ? "0" : number.substr(first);
}
bool ArchiveOrder(const io::Entry& a, const io::Entry& b) {
    const auto ap = PatchNumber(a.name), bp = PatchNumber(b.name);
    if (bool(ap) != bool(bp)) return bool(ap);
    if (ap && bp && *ap != *bp) return ap->size() != bp->size() ? ap->size() > bp->size() : *ap > *bp;
    const auto an = Lower(a.name), bn = Lower(b.name);
    if ((an == "script.noa") != (bn == "script.noa")) return an == "script.noa";
    return an != bn ? an < bn : a.name < b.name;
}
} // namespace

std::optional<GameLaunchConfig> DiscoverArchiveFallback(const fs::path& root) {
    try {
        const auto files = io::List(root);
        std::vector<io::Entry> archives;
        std::optional<fs::path> scriptArchive;
        for (const auto& file : files) {
            if (file.info.kind != io::FileInfo::Kind::File || file.info.symbolicLink ||
                Lower(fs::u8path(file.name).extension().u8string()) != ".noa") continue;
            archives.push_back(file);
            if (Lower(file.name) == "script.noa") {
                if (scriptArchive) Invalid(root, "multiple script.noa filenames differ only by case");
                scriptArchive = root / fs::u8path(file.name);
            }
        }
        if (!scriptArchive) return {};
        if (archives.size() > maxArchives) Invalid(root, "too many NOA archives; supply entis-launcher.xml");
        std::size_t totalIndices = 0;
        const auto scriptIndex = ReadIndex(*scriptArchive, totalIndices);
        if (!scriptIndex.script) return {};
        ValidateScript(*scriptArchive, *scriptIndex.script);
        std::sort(archives.begin(), archives.end(), ArchiveOrder);
        std::string mounts, font, verticalFont, effectiveScript;
        for (const auto& archive : archives) {
            const auto path = root / fs::u8path(archive.name);
            const auto index = path == *scriptArchive ? scriptIndex : ReadIndex(path, totalIndices);
            mounts += "<archive path=\"" + Escape(archive.name) + "\"/>\n";
            if (effectiveScript.empty() && index.script) {
                if (path != *scriptArchive) ValidateScript(path, *index.script);
                effectiveScript = index.script->name;
            }
            if (font.empty() && index.font) font = index.font->name;
            if (verticalFont.empty() && index.verticalFont) verticalFont = index.verticalFont->name;
        }
        for (const auto& file : files) {
            if (file.info.kind != io::FileInfo::Kind::File || file.info.symbolicLink) continue;
            if (font.empty() && Lower(file.name) == "msgfont.bmf") font = file.name;
            if (verticalFont.empty() && Lower(file.name) == "msgfont_v.bmf") verticalFont = file.name;
        }
        std::string xml = "<script src=\"" + Escape(effectiveScript) + "\">\n" + mounts + "<file path=\".\"/>\n";
        if (!font.empty() || !verticalFont.empty()) {
            xml += "<fonts>\n";
            if (!font.empty()) xml += "<file name=\"MsgFont\" path=\"" + Escape(font) + "\"/>\n";
            if (!verticalFont.empty()) xml += "<file name=\"@MsgFont\" path=\"" + Escape(verticalFont) + "\"/>\n";
            xml += "</fonts>\n";
        }
        // Missing display/audio/module settings cannot be inferred from an
        // archive index. Keep engine defaults and let explicit XML override.
        xml += "</script>\n";
        auto result = NormalizeGameConfig(root, xml, scriptArchive->filename().u8string() + ":script.csx (fallback)");
        result.warnings.push_back("No launch manifest was available; using script.noa/script.csx with inferred archive order, standard bitmap fonts and engine defaults. Supply entis-launcher.xml if this game needs different settings.");
        return result;
    } catch (const ConfigError&) { throw; }
    catch (const std::exception& error) {
        throw ConfigError("Archive fallback: " + std::string(error.what()));
    }
}
} // namespace entis::launcher
