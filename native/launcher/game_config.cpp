#include "launcher/game_config.h"
#include "io/game_files.h"

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <map>
#include <set>
#include <sstream>

namespace entis::launcher {
namespace {
namespace fs = std::filesystem;

constexpr std::size_t maxXml = 1024 * 1024;
constexpr std::size_t maxExecutable = 256 * 1024 * 1024;

std::string Lower(std::string value) {
    for (char& c : value) if (static_cast<unsigned char>(c) < 128)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return value;
}
// A deliberately small, bounded XML subset for launch configuration. It uses
// standard containers so discovery can precede SDK global/platform setup.
struct XmlNode {
    std::string tag;
    std::map<std::string, std::string> attributes;
    std::vector<std::unique_ptr<XmlNode>> children;
    enum { typeRoot, typeTag, typeComment };
    int GetType() const { return tag.empty() ? typeRoot : typeTag; }
    const std::string& GetTag() const { return tag; }
    void SetTag(const std::string& value) { tag = value; }
    void SetAttributeAs(const std::string& key, const std::string& value) { attributes[key] = value; }
    const std::string* GetAttributeAs(const std::string& key) const {
        const auto found = attributes.find(key);
        return found == attributes.end() ? nullptr : &found->second;
    }
    std::string GetAttrStringAs(const std::string& key) const {
        const auto* value = GetAttributeAs(key); return value ? *value : std::string();
    }
    std::size_t GetAttributeCount() const { return attributes.size(); }
    const std::string* GetAttributeNameAt(std::size_t index) const {
        auto it=attributes.begin(); std::advance(it,index); return &it->first;
    }
    const std::string* GetAttributeValueAt(std::size_t index) const {
        auto it=attributes.begin(); std::advance(it,index); return &it->second;
    }
    std::size_t GetElementsCount() const { return children.size(); }
    XmlNode* GetElementAt(std::size_t index) const { return children.at(index).get(); }
    void AddElement(XmlNode* node) { children.emplace_back(node); }
    XmlNode* CreateElementTagAs(const std::string& value) {
        auto node=std::make_unique<XmlNode>(); node->tag=value;
        auto* result=node.get(); children.push_back(std::move(node)); return result;
    }
};
std::string Utf8(const std::string& text) { return text; }
std::string Attr(const XmlNode& doc, const char* name) { return doc.GetAttrStringAs(name); }
void Set(XmlNode& doc, const char* name, const std::string& value) { doc.SetAttributeAs(name,value); }
void AppendCodepoint(std::string& out, uint32_t code) {
    if (code == 0 || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff) ||
        (code < 32 && code != 9 && code != 10 && code != 13)) throw ConfigError("Invalid XML character reference");
    if (code < 0x80) out += char(code);
    else if (code < 0x800) { out += char(0xc0 | (code >> 6)); out += char(0x80 | (code & 63)); }
    else if (code < 0x10000) { out += char(0xe0 | (code >> 12)); out += char(0x80 | ((code >> 6)&63)); out += char(0x80 | (code&63)); }
    else { out += char(0xf0 | (code >> 18)); out += char(0x80 | ((code >> 12)&63)); out += char(0x80 | ((code >> 6)&63)); out += char(0x80 | (code&63)); }
}
std::string CleanUtf8(const std::string& source, bool& repaired) {
    std::string out;
    out.reserve(source.size());
    for (std::size_t p=0;p<source.size();) {
        const auto first=static_cast<unsigned char>(source[p]);
        if (first<128) {
            if (first<32 && first!=9 && first!=10 && first!=13)
                throw ConfigError("Control character in XML configuration");
            out+=source[p++]; continue;
        }
        const unsigned count=first>=0xc2 && first<=0xdf ? 2 : first>=0xe0 && first<=0xef ? 3 : first>=0xf0 && first<=0xf4 ? 4 : 0;
        bool valid=count && count<=source.size()-p;
        uint32_t code=count ? first & ((1u<<(7-count))-1) : 0;
        for(unsigned i=1;valid && i<count;++i) {
            const auto next=static_cast<unsigned char>(source[p+i]);
            valid=(next & 0xc0)==0x80;
            code=(code<<6)|(next&63);
        }
        valid=valid && code>= (count==2 ? 0x80u : count==3 ? 0x800u : 0x10000u)
            && code<=0x10ffff && !(code>=0xd800 && code<=0xdfff);
        if (valid) { out.append(source,p,count); p+=count; }
        else { AppendCodepoint(out,0xfffd); ++p; repaired=true; }
    }
    return out;
}
std::string DecodeAttribute(const std::string& text) {
    std::string out;
    for (std::size_t i=0;i<text.size();++i) {
        if (text[i]!='&') { out += text[i]; continue; }
        const auto end=text.find(';',i+1);
        if (end==std::string::npos) throw ConfigError("Unterminated XML entity");
        const auto value=text.substr(i+1,end-i-1);
        if (value=="amp") out+='&'; else if (value=="lt") out+='<';
        else if (value=="gt") out+='>'; else if (value=="quot") out+='\"'; else if (value=="apos") out+='\'';
        else if (!value.empty() && value.front()=='#') {
            const bool hex=value.size()>1 && value[1]=='x';
            const auto digits=value.substr(hex ? 2 : 1);
            if (digits.empty() || digits.size()>8) throw ConfigError("Invalid numeric XML entity");
            uint32_t code=0;
            for(const char c:digits) {
                const unsigned d=c>='0' && c<='9' ? c-'0' : hex && c>='a' && c<='f' ? c-'a'+10 : hex && c>='A' && c<='F' ? c-'A'+10 : 99;
                if (d >= (hex ? 16u : 10u)) throw ConfigError("Invalid numeric XML entity");
                code=code*(hex ? 16 : 10)+d;
            }
            AppendCodepoint(out,code);
        } else throw ConfigError("Unsupported XML entity: &"+value+";");
        i=end;
    }
    return out;
}
std::string Escape(const std::string& value) {
    std::string out;
    for (const char c:value) {
        if(c=='&')out+="&amp;"; else if(c=='<')out+="&lt;"; else if(c=='>')out+="&gt;";
        else if(c=='\"')out+="&quot;"; else out+=c;
    }
    return out;
}
void Serialize(const XmlNode& node, std::string& out) {
    if(node.tag.empty()) { for(const auto& child:node.children) Serialize(*child,out); return; }
    out+="<"+node.tag;
    for(const auto& attr:node.attributes) out+=" "+attr.first+"=\""+Escape(attr.second)+"\"";
    if(node.children.empty()) { out+="/>\n"; return; }
    out+=">\n";
    for(const auto& child:node.children) Serialize(*child,out);
    out+="</"+node.tag+">\n";
}
std::vector<uint8_t> ReadFile(const fs::path& path, std::size_t limit) {
    try {
        auto bytes = io::ReadFile(path, limit);
        if (bytes.empty()) throw ConfigError("Configuration source is empty or too large: " + path.u8string());
        return bytes;
    } catch (const ConfigError&) { throw; }
    catch (const std::exception& error) {
        throw ConfigError("Cannot read configuration source: " + path.u8string() + ": " + error.what());
    }
}
fs::path Root(const fs::path& path) {
    try {
        const auto root = io::Canonical(path);
        if (io::Stat(root).kind != io::FileInfo::Kind::Directory)
            throw ConfigError("Game directory does not exist: " + path.u8string());
        return root;
    } catch (const std::exception& error) {
        throw ConfigError("Game directory does not exist: " + path.u8string() + ": " + error.what());
    }
}
void CheckContained(const fs::path& root, const fs::path& path) {
    fs::path full;
    try { full = io::Canonical(path, false); }
    catch (const std::exception& error) { throw ConfigError("Cannot resolve game path: " + path.u8string() + ": " + error.what()); }
    auto r = root.begin(), p = full.begin();
    for (; r != root.end(); ++r, ++p)
        if (p == full.end() || *r != *p)
            throw ConfigError("Configuration path escapes the selected game directory: " + path.u8string());
}

// Unlike the permissive SDK reader, require matched tags and quoted attributes,
// reject DTDs, and bound resource usage before passing normalized XML to the VM.
XmlNode ParseXml(const std::string& xml) {
    XmlNode document;
    std::vector<XmlNode*> nodes{&document};
    if (xml.empty() || xml.size() > maxXml) throw ConfigError("XML configuration is empty or exceeds 1 MiB");
    if (xml.find('\0') != std::string::npos) throw ConfigError("Configuration must be UTF-8 XML without NUL bytes");
    std::size_t p = xml.compare(0, 3, "\xef\xbb\xbf") == 0 ? 3 : 0;
    std::vector<std::string> stack;
    unsigned roots = 0, elements = 0;
    auto space = [&] { while (p < xml.size() && std::isspace(static_cast<unsigned char>(xml[p]))) ++p; };
    auto name = [&] {
        const auto begin = p;
        while (p < xml.size()) {
            const unsigned char c = xml[p];
            if (!(std::isalnum(c) || c == '_' || c == '-' || c == ':' || c == '.')) break;
            ++p;
        }
        if (begin == p) throw ConfigError("Invalid XML element or attribute name");
        return xml.substr(begin, p - begin);
    };
    while (p < xml.size()) {
        space();
        if (p == xml.size()) break;
        if (xml[p] != '<') throw ConfigError("Only configuration elements are supported; unexpected text");
        if (xml.compare(p, 4, "<!--") == 0) {
            const auto end = xml.find("-->", p + 4);
            if (end == std::string::npos) throw ConfigError("Unclosed XML comment");
            p = end + 3; continue;
        }
        if (xml.compare(p, 2, "<?") == 0) {
            if (xml.compare(p, 5, "<?xml") || roots)
                throw ConfigError("Unsupported XML processing instruction");
            const auto end = xml.find("?>", p + 5);
            if (end == std::string::npos) throw ConfigError("Unclosed XML declaration");
            const auto declaration = Lower(xml.substr(p, end - p));
            if (declaration.find("encoding") != std::string::npos &&
                declaration.find("utf-8") == std::string::npos && declaration.find("utf8") == std::string::npos)
                throw ConfigError("Only UTF-8 launch configurations are supported");
            p = end + 2; continue;
        }
        if (xml.compare(p, 2, "<!") == 0) throw ConfigError("DTD, entities and CDATA are unsupported in launch configuration");
        ++p;
        if (p < xml.size() && xml[p] == '/') {
            ++p; const auto closing = name(); space();
            if (p == xml.size() || xml[p++] != '>' || stack.empty() || closing != stack.back())
                throw ConfigError("Mismatched XML closing element: " + closing);
            stack.pop_back(); nodes.pop_back(); continue;
        }
        const auto tag = name();
        auto* parsedNode=nodes.back()->CreateElementTagAs(tag);
        if (++elements > 4096 || stack.size() >= 16) throw ConfigError("XML configuration is too complex");
        if (stack.empty() && ++roots != 1) throw ConfigError("Expected exactly one XML root element");
        std::set<std::string> attrs;
        bool closed = false;
        for (;;) {
            space();
            if (p >= xml.size()) throw ConfigError("Unclosed XML element: " + tag);
            if (xml.compare(p, 2, "/>") == 0) { p += 2; closed = true; break; }
            if (xml[p] == '>') { ++p; break; }
            const auto attribute = name();
            if (!attrs.insert(attribute).second) throw ConfigError("Duplicate XML attribute: " + attribute);
            if (attrs.size()>128) throw ConfigError("Too many XML attributes");
            space();
            if (p >= xml.size() || xml[p++] != '=') throw ConfigError("Expected XML attribute assignment");
            space();
            if (p >= xml.size() || (xml[p] != '\'' && xml[p] != '"')) throw ConfigError("XML attributes must be quoted");
            const char quote = xml[p++];
            const auto end = xml.find(quote, p);
            if (end == std::string::npos || xml.find('<', p) < end) throw ConfigError("Invalid XML attribute value");
            if (end-p>16384) throw ConfigError("XML attribute is too long");
            parsedNode->SetAttributeAs(attribute,DecodeAttribute(xml.substr(p,end-p)));
            p = end + 1;
        }
        if (!closed) { stack.push_back(tag); nodes.push_back(parsedNode); }
    }
    if (!stack.empty() || roots != 1) throw ConfigError("Incomplete XML configuration");
    return document;
}

std::string Path(const fs::path& root, std::string path, bool hostPath, bool allowRoot = false) {
    std::replace(path.begin(), path.end(), '\\', '/');
    bool explicitGame = false;
    if (path == "$(CURRENT)" || path.rfind("$(CURRENT)/", 0) == 0) {
        path.erase(0, 10); explicitGame = true;
        if (!path.empty() && path.front() == '/') path.erase(0, 1);
    } else if (path == "storage://game" || path.rfind("storage://game/", 0) == 0) {
        path.erase(0, 14); explicitGame = true;
        if (!path.empty() && path.front() == '/') path.erase(0, 1);
    }
    if ((!path.empty() && path.front() == '/') || path.find(':') != std::string::npos || path.find("$(") != std::string::npos)
        throw ConfigError("Only game-relative paths are supported: " + path);
    for (const unsigned char c : path)
        if (c < 32 || c == 127) throw ConfigError("Control character in configuration path");
    std::string normalized;
    std::istringstream parts(path);
    std::string part;
    while (std::getline(parts, part, '/')) {
        if (part.empty() || part == ".") continue;
        if (part == "..") throw ConfigError("Parent traversal is not allowed in configuration path: " + path);
        if (!normalized.empty()) normalized += '/';
        normalized += part;
    }
    if (normalized.empty() && !allowRoot) throw ConfigError("Empty resource path in configuration");
    CheckContained(root, root / fs::u8path(normalized));
    if (hostPath || explicitGame) return normalized.empty() ? "storage://game" : "storage://game/" + normalized;
    return normalized;
}
void CopyAttributes(const XmlNode& from, XmlNode& to,
                    std::initializer_list<const char*> allowed, GameLaunchConfig& result) {
    for (std::size_t i = 0; i < from.GetAttributeCount(); ++i) {
        const auto* name = from.GetAttributeNameAt(i);
        if (!name) continue;
        bool found = false;
        for (const auto* key : allowed) if (*name == key) { found = true; break; }
        if (!found) throw ConfigError("Unsupported attribute <" + Utf8(from.GetTag()) + " " + Utf8(*name) + "> in " + result.configSource);
        to.SetAttributeAs(*name, *from.GetAttributeValueAt(i));
    }
}
void NoChildren(const XmlNode& node) {
    for (std::size_t i = 0; i < node.GetElementsCount(); ++i) {
        const auto* child = node.GetElementAt(i);
        if (child && child->GetType() != XmlNode::typeComment)
            throw ConfigError("Unsupported nested configuration element: " + Utf8(node.GetTag()));
    }
}
std::string Identifier(const std::string& value, const char* field) {
    if (value.empty() || value.size() > 80) throw ConfigError(std::string("Invalid ") + field);
    for (const unsigned char c : value)
        if (!(std::isalnum(c) && c < 128) && c != '-' && c != '_' && c != '.')
            throw ConfigError(std::string("Invalid ") + field + ": " + value);
    const auto base=Lower(value.substr(0,value.find('.')));
    const bool reserved=base=="con" || base=="prn" || base=="aux" || base=="nul" ||
        (base.size()==4 && (base.rfind("com",0)==0 || base.rfind("lpt",0)==0) && base[3]>='1' && base[3]<='9');
    if (value == "." || value == ".." || value.back()=='.' || reserved)
        throw ConfigError(std::string("Invalid ") + field);
    return value;
}
std::string Fingerprint(const std::string& xml) {
    // An identity for data separation, not a cryptographic integrity check.
    uint64_t hash = UINT64_C(14695981039346656037);
    for (const unsigned char byte : xml) { hash ^= byte; hash *= UINT64_C(1099511628211); }
    std::ostringstream out;
    out << "game-" << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}
void AddFileIdentity(std::string& identity, const fs::path& path) {
    try {
        const auto info = io::Stat(path);
        if (info.kind != io::FileInfo::Kind::File) return;
        const auto bytes = io::ReadPrefix(path, static_cast<std::size_t>(std::min<std::uint64_t>(info.size, 64 * 1024)));
        const std::string prefix(bytes.begin(), bytes.end());
        // NOA headers/index prefixes and direct modules distinguish applications
        // whose minimal XML is otherwise identical. No host path enters this ID.
        identity += "\nresource-size:" + std::to_string(info.size) + "\nprefix:" + Fingerprint(prefix);
    } catch (const std::exception& error) {
        throw ConfigError("Cannot read resource identity: " + path.u8string() + ": " + error.what());
    }
}

class MemoryInput final : public SSystem::SInputStream {
    const std::vector<uint8_t>& data;
    std::size_t offset = 0;
public:
    explicit MemoryInput(const std::vector<uint8_t>& bytes) : data(bytes) {}
    size_t Read(void* output, size_t count) override {
        count = std::min(count, data.size() - offset);
        if (count) std::memcpy(output, data.data() + offset, count);
        offset += count;
        return count;
    }
};
std::string DecodeConfig(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) throw ConfigError("Empty IDR_COTOMI resource");
    std::size_t p = bytes.size() >= 3 && bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf ? 3 : 0;
    while (p < bytes.size() && std::isspace(bytes[p])) ++p;
    if (p < bytes.size() && bytes[p] == '<') {
        std::string output(bytes.begin(),bytes.end());
        while(!output.empty() && output.back()=='\0') output.pop_back();
        return output;
    }
    MemoryInput input(bytes);
    ERISA::SGLDecodeBitStream bits(0x4000);
    bits.AttachInputStream(&input);
    ERISA::SGLERISANDecodeContext decoder(&bits);
    decoder.PrepareToDecodeERISANCode();
    std::string output;
    std::array<char, 4096> block;
    while (output.size() <= maxXml) {
        const auto got = decoder.Read(block.data(), std::min(block.size(), maxXml + 1 - output.size()));
        if (!got) break;
        output.append(block.data(), got);
    }
    if (output.empty() || output.size() > maxXml) throw ConfigError("Invalid or oversized ERISAN IDR_COTOMI resource");
    // Native Windows resources sometimes include their string terminator.
    while (!output.empty() && output.back() == '\0') output.pop_back();
    return output;
}

// PE32 and PE32+ resource reader. All offsets are checked against file-backed
// section bytes; virtual section tails are not readable file data.
std::vector<std::vector<uint8_t>> Resources(const std::vector<uint8_t>& bytes) {
    auto check = [&](std::size_t p, std::size_t n) {
        if (p > bytes.size() || n > bytes.size() - p) throw ConfigError("Truncated PE resource data");
    };
    auto u16 = [&](std::size_t p) { check(p, 2); return unsigned(bytes[p]) | unsigned(bytes[p+1]) << 8; };
    auto u32 = [&](std::size_t p) { check(p, 4); return uint32_t(bytes[p]) | uint32_t(bytes[p+1]) << 8 | uint32_t(bytes[p+2]) << 16 | uint32_t(bytes[p+3]) << 24; };
    check(0, 64);
    if (bytes[0] != 'M' || bytes[1] != 'Z') throw ConfigError("Not a Windows PE executable");
    const std::size_t pe = u32(60);
    check(pe, 24);
    if (u32(pe) != 0x00004550) throw ConfigError("Invalid Windows PE signature");
    const auto sections = u16(pe + 6), optionalSize = u16(pe + 20);
    const std::size_t optional = pe + 24;
    check(optional, optionalSize);
    const auto magic = u16(optional);
    const std::size_t directories = magic == 0x10b ? 96 : magic == 0x20b ? 112 : 0;
    if (!directories || optionalSize < directories + 24) throw ConfigError("Unsupported PE optional header");
    if (u32(optional + directories - 4) < 3) return {};
    if (sections == 0 || sections > 96) throw ConfigError("Invalid PE section count");
    const std::size_t sectionTable = optional + optionalSize;
    check(sectionTable, std::size_t(sections) * 40);
    auto rva = [&](uint32_t address, std::size_t length) -> std::size_t {
        for (unsigned i = 0; i < sections; ++i) {
            const auto section = sectionTable + i * 40;
            const auto start = u32(section + 12), rawSize = u32(section + 16), rawOffset = u32(section + 20);
            if (address >= start && uint64_t(address) - start < rawSize) {
                const uint64_t delta = uint64_t(address) - start;
                if (length > rawSize - delta) throw ConfigError("PE resource crosses section boundary");
                const uint64_t fileOffset = uint64_t(rawOffset) + delta;
                if (fileOffset > std::numeric_limits<std::size_t>::max()) throw ConfigError("PE resource offset overflow");
                check(static_cast<std::size_t>(fileOffset), length);
                return static_cast<std::size_t>(fileOffset);
            }
        }
        throw ConfigError("PE resource RVA has no file-backed section");
    };
    const auto resourceRva = u32(optional + directories + 16), resourceSize = u32(optional + directories + 20);
    if (!resourceRva || !resourceSize) return {};
    const auto base = rva(resourceRva, resourceSize);
    auto relative = [&](uint32_t offset, std::size_t length) {
        if (offset > resourceSize || length > resourceSize - offset) throw ConfigError("Invalid PE resource directory offset");
        check(base + offset, length); return base + offset;
    };
    std::vector<std::vector<uint8_t>> results;
    std::set<uint32_t> visited;
    std::function<void(uint32_t, unsigned, bool)> walk = [&](uint32_t offset, unsigned depth, bool selected) {
        if (depth > 3 || !visited.insert(offset).second) throw ConfigError("Cyclic or over-nested PE resource directory");
        const auto directory = relative(offset, 16);
        const auto count = u16(directory + 12) + u16(directory + 14);
        if (count > 4096) throw ConfigError("Too many PE resource entries");
        relative(offset + 16, std::size_t(count) * 8);
        for (unsigned i = 0; i < count; ++i) {
            const auto name = u32(directory + 16 + i * 8), entry = u32(directory + 20 + i * 8);
            bool match = selected;
            if (depth == 0) match = name == 10; // RT_RCDATA.
            if (depth == 1) {
                match = false;
                if (name & 0x80000000u) {
                    const auto nameOffset = name & 0x7fffffffu;
                    const auto nameStart = relative(nameOffset, 2);
                    const auto length = u16(nameStart);
                    relative(nameOffset + 2, std::size_t(length) * 2);
                    std::string text;
                    for (unsigned n = 0; n < length; ++n) {
                        const auto c = u16(nameStart + 2 + n * 2);
                        text += c < 128 ? static_cast<char>(c) : '?';
                    }
                    match = selected && text == "IDR_COTOMI";
                }
            }
            if (!match) continue;
            if (entry & 0x80000000u) walk(entry & 0x7fffffffu, depth + 1, match);
            else if (depth >= 2) {
                const auto leaf = relative(entry, 16);
                const auto address = u32(leaf), length = u32(leaf + 4);
                if (length == 0 || length > maxXml * 4) throw ConfigError("Invalid IDR_COTOMI resource size");
                const auto data = rva(address, length);
                std::vector<uint8_t> value(bytes.begin() + data, bytes.begin() + data + length);
                if (std::find(results.begin(), results.end(), value) == results.end()) results.push_back(std::move(value));
            }
        }
    };
    walk(0, 0, false);
    return results;
}
} // namespace

GameLaunchConfig NormalizeGameConfig(const fs::path& gameDir, const std::string& xml, const std::string& source) {
    const auto root = Root(gameDir);
    if(xml.size()>maxXml) throw ConfigError("XML configuration exceeds 1 MiB");
    bool repairedUtf8=false;
    XmlNode document = ParseXml(CleanUtf8(xml,repairedUtf8));
    XmlNode* script = nullptr;
    for (std::size_t i = 0; i < document.GetElementsCount(); ++i) {
        auto* node = document.GetElementAt(i);
        if (!node || node->GetType() == XmlNode::typeComment) continue;
        if (node->GetTag() != "script" && node->GetTag() != "cotopha")
            throw ConfigError("Unsupported launch document; expected <script> or <cotopha>");
        if (script) throw ConfigError("Ambiguous launch document");
        script = node;
    }
    if (!script) throw ConfigError("Launch configuration has no script element");
    GameLaunchConfig result;
    result.configSource = source;
    if(repairedUtf8) result.warnings.push_back("Invalid UTF-8 in launch configuration was replaced with U+FFFD");
    result.entryScript = Path(root, Attr(*script, "src"), false);
    if (Lower(fs::u8path(result.entryScript).extension().u8string()) != ".csx")
        throw ConfigError("Unsupported entry format '" + result.entryScript + "': this runtime currently supports traditional Cotopha .csx only");
    result.compatibilityProfile = Attr(*script, "profile");
    if (!result.compatibilityProfile.empty()) Identifier(result.compatibilityProfile, "compatibility profile");
    const auto explicitId = Attr(*script, "id");
    result.explicitGameId = !explicitId.empty();
    if (!explicitId.empty()) Identifier(explicitId, "game id");
    const auto key = Attr(*script, "psb_key");
    if (!key.empty()) {
        std::size_t consumed = 0;
        try {
            const bool hex = key.rfind("0x", 0) == 0 || key.rfind("0X", 0) == 0;
            const auto digits = key.substr(hex ? 2 : 0);
            if (digits.empty() || digits.find_first_not_of(hex ? "0123456789abcdefABCDEF" : "0123456789") != std::string::npos)
                throw std::out_of_range("key");
            const auto value = std::stoull(key, &consumed, hex ? 16 : 10);
            if (consumed != key.size() || value > UINT32_MAX) throw std::out_of_range("key");
            result.psbKey = static_cast<uint32_t>(value);
        } catch (...) { throw ConfigError("psb_key must be a 32-bit unsigned decimal or 0x hexadecimal integer"); }
    }
    XmlNode normalized;
    auto* output = normalized.CreateElementTagAs("script");
    CopyAttributes(*script, *output, {"src", "profile", "id", "psb_key"}, result);
    // This is a launcher setting, not an SDK environment attribute or game
    // identity. Changing or removing a key must not select a different save.
    output->attributes.erase("psb_key");
    Set(*output, "src", result.entryScript);
    // Keep the historic path while computing identity below. The emitted
    // runtime path changes to the selected game directory after hashing so a
    // storage-policy change cannot select a different game's settings/saves.
    auto* save = new XmlNode;
    save->SetTag("save_dir"); save->SetAttributeAs("path", "local://savedata"); output->AddElement(save);
    unsigned displayCount = 0;
    std::vector<fs::path> archiveFiles;
    std::vector<fs::path> fileDirectories;
    for (std::size_t i = 0; i < script->GetElementsCount(); ++i) {
        const auto* node = script->GetElementAt(i);
        if (!node || node->GetType() == XmlNode::typeComment) continue;
        const auto tag = Utf8(node->GetTag());
        if (tag == "save_dir" || tag == "icon") continue;
        if (tag == "module") throw ConfigError("Windows DLL modules are unsupported; this game needs a native compatibility module");
        if (tag != "archive" && tag != "file" && tag != "fonts" && tag != "display" && tag != "vm" && tag != "sound" && tag != "opengl")
            throw ConfigError("Unsupported launch configuration element: <" + tag + ">");
        auto owned = std::make_unique<XmlNode>();
        auto& out = *owned;
        out.SetTag(node->GetTag());
        if (tag == "archive" || tag == "file") {
            NoChildren(*node);
            if (tag == "archive") CopyAttributes(*node, out, {"path", "id", "key", "default_dir", "fragment", "fragment_cache", "encrypt32"}, result);
            else CopyAttributes(*node, out, {"path", "id", "fragment", "fragment_cache"}, result);
            const auto path = Path(root, Attr(*node, "path"), true, tag == "file");
            Set(out, "path", path);
            if (node->GetAttributeAs("default_dir")) Set(out, "default_dir", Path(root, Attr(*node, "default_dir"), false, true));
            const auto relative = path == "storage://game" ? "" : path.substr(15);
            if (io::Stat(root / fs::u8path(relative)).kind == io::FileInfo::Kind::Missing) result.warnings.push_back("Configured " + tag + " does not exist: " + relative);
            if (tag == "archive") archiveFiles.push_back(root / fs::u8path(relative));
            else fileDirectories.push_back(root / fs::u8path(relative));
        } else if (tag == "fonts") {
            CopyAttributes(*node, out, {}, result);
            for (std::size_t j = 0; j < node->GetElementsCount(); ++j) {
                const auto* font = node->GetElementAt(j);
                if (!font || font->GetType() == XmlNode::typeComment) continue;
                NoChildren(*font);
                auto next = std::make_unique<XmlNode>(); next->SetTag(font->GetTag());
                if (font->GetTag() == "file") {
                    CopyAttributes(*font, *next, {"name", "path", "cache_kb"}, result);
                    const auto path = Path(root, Attr(*font, "path"), false);
                    Set(*next, "path", path);
                    const auto extension = Lower(fs::u8path(path).extension().u8string());
                    if (extension == ".ttf" || extension == ".otf" || extension == ".ttc" || extension == ".otc") {
                        result.openTypeFonts.push_back({path, Attr(*font, "name")});
                        continue; // The SDK file loader only understands bitmap fonts.
                    }
                    if (extension != ".bmf") throw ConfigError("Unsupported configured font format: " + path);
                    if (Attr(*font, "name").empty()) throw ConfigError("Bitmap fonts require a name attribute");
                } else if (font->GetTag() == "filter") {
                    CopyAttributes(*font, *next, {"in", "out"}, result);
                    const auto alias = Attr(*font, "in"), target = Attr(*font, "out");
                    if (alias.empty() || target.empty()) throw ConfigError("Font filter requires in/out attributes");
                    result.fontAliases.push_back({alias, target});
                    continue; // Apply only after all portable fonts have loaded.
                } else throw ConfigError("Unsupported <fonts> element: " + Utf8(font->GetTag()));
                out.AddElement(next.release());
            }
        } else {
            NoChildren(*node);
            if (tag == "display") {
                if (++displayCount > 1) throw ConfigError("Multiple display configurations are unsupported");
                CopyAttributes(*node, out, {"caption", "width", "height", "depth", "frequency", "CooperationLevel", "change_mode"}, result);
                result.title = Attr(*node, "caption");
            } else if (tag == "sound") CopyAttributes(*node, out, {"frequency", "channels", "bits_per_sample"}, result);
            else if (tag == "opengl") CopyAttributes(*node, out, {"any_thread", "non_power_of_2", "element_index_uint", "gl2_0", "std_gouraud_shader", "std_phong_shader", "compute_shader", "limit_bone", "limit_light", "limit_shadowmap", "env_mapping", "env_cube_mapping", "env_sphere_mapping", "env_viewport_mapping", "env_refraction", "normal_mapping", "height_mapping", "specular_mapping", "global_ao_lightmap"}, result);
            else {
                CopyAttributes(*node, out, {"jit_compiler", "jit_boundary", "max_heap_block", "heap_size", "init_stack_size", "x86", "arm"}, result);
                out.SetAttributeAs("jit_compiler", "false");
            }
        }
        output->AddElement(owned.release());
    }
    std::map<std::string,std::string> remaps;
    for(const auto& alias:result.fontAliases) {
        if(!remaps.emplace(alias.alias,alias.source).second)
            throw ConfigError("Duplicate font filter: "+alias.alias);
    }
    std::vector<FontAlias> orderedAliases;
    std::map<std::string,unsigned> visit;
    for(const auto& alias:result.fontAliases) {
        std::vector<std::string> chain;
        std::string current=alias.alias;
        while(remaps.count(current) && visit[current]!=2) {
            if(visit[current]==1)throw ConfigError("Cyclic font filters: "+current);
            visit[current]=1;
            chain.push_back(current);
            current=remaps.at(current);
        }
        for(auto it=chain.rbegin();it!=chain.rend();++it) {
            orderedAliases.push_back({*it,remaps.at(*it)});
            visit[*it]=2;
        }
    }
    result.fontAliases=std::move(orderedAliases);
    result.normalizedXml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    Serialize(normalized,result.normalizedXml);
    // Include the separately loaded fonts in identity as they are omitted from
    // the SDK document to avoid invoking its bitmap-only file loader.
    std::string identity = result.normalizedXml;
    for (const auto& font : result.openTypeFonts) identity += "\nfont:" + font.path + "\nname:" + font.family;
    for (const auto& alias : result.fontAliases) identity += "\nalias:" + alias.alias + "\ntarget:" + alias.source;
    for (const auto& archive : archiveFiles) AddFileIdentity(identity, archive);
    const auto entry = result.entryScript.rfind("storage://game/", 0) == 0
        ? result.entryScript.substr(15) : result.entryScript;
    AddFileIdentity(identity, root / fs::u8path(entry));
    for (const auto& directory : fileDirectories) {
        const auto candidate = directory / fs::u8path(entry);
        CheckContained(root, candidate);
        if (candidate != root / fs::u8path(entry)) AddFileIdentity(identity, candidate);
    }
    result.gameId = explicitId.empty() ? Fingerprint(identity) : explicitId;
    if (explicitId.empty() && script->GetAttributeAs("psb_key")) {
        output->attributes["psb_key"] = key;
        std::string previousXml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
        Serialize(normalized, previousXml);
        const auto previous = Fingerprint(previousXml + identity.substr(result.normalizedXml.size()));
        if (previous != result.gameId) result.previousGameId = previous;
    }
    save->SetAttributeAs("path", "storage://game/savedata");
    result.normalizedXml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    output->attributes.erase("psb_key");
    Serialize(normalized, result.normalizedXml);
    if (result.title.empty()) result.title = root.filename().u8string();
    return result;
}

GameLaunchConfig DiscoverGame(const fs::path& gameDir, const fs::path& explicitConfig) {
    const auto root = Root(gameDir);
    auto read = [&](const fs::path& path) {
        CheckContained(root, path);
        if (Lower(path.extension().u8string()) == ".exe") {
            const auto resources = Resources(ReadFile(path, maxExecutable));
            if (resources.empty()) throw ConfigError("Executable has no RCDATA/IDR_COTOMI launch configuration: " + path.u8string());
            if (resources.size() != 1) throw ConfigError("Executable contains multiple different IDR_COTOMI configurations; provide entis-launcher.xml");
            return NormalizeGameConfig(root, DecodeConfig(resources.front()), path.filename().u8string() + ":IDR_COTOMI");
        }
        const auto bytes = ReadFile(path, maxXml);
        return NormalizeGameConfig(root, std::string(bytes.begin(), bytes.end()), path.filename().u8string());
    };
    if (!explicitConfig.empty()) return read(explicitConfig.is_absolute() ? explicitConfig : root / explicitConfig);
    for (const char* name : {"entis-launcher.xml", "cotopha.xml"}) {
        const auto candidate = root / name;
        if (io::Stat(candidate).kind != io::FileInfo::Kind::Missing) return read(candidate);
    }
    std::vector<fs::path> candidates;
    std::vector<std::string> scanErrors;
    unsigned count = 0;
    for (const auto& entry : io::List(root)) {
        const auto path = root / fs::u8path(entry.name);
        if (entry.info.kind != io::FileInfo::Kind::File || Lower(path.extension().u8string()) != ".exe") continue;
        if (++count > 128) throw ConfigError("Too many executables to discover automatically; provide entis-launcher.xml");
        try {
            CheckContained(root, path);
            if (!Resources(ReadFile(path, maxExecutable)).empty()) candidates.push_back(path);
        } catch (const ConfigError& error) { scanErrors.push_back(error.what()); }
    }
    std::sort(candidates.begin(), candidates.end());
    if (candidates.empty()) {
        std::string message = "No launch configuration found. Supply entis-launcher.xml, cotopha.xml, or the original Windows EXE containing IDR_COTOMI alongside game resources.";
        if (!scanErrors.empty()) message += " First EXE read error: " + scanErrors.front();
        throw ConfigError(message);
    }
    if (candidates.size() != 1) {
        std::string message = "Multiple game executables contain launch configurations; select one explicitly or supply entis-launcher.xml:";
        for (const auto& path : candidates) message += " " + path.filename().u8string();
        throw ConfigError(message);
    }
    auto result = read(candidates.front());
    result.warnings.insert(result.warnings.end(), scanErrors.begin(), scanErrors.end());
    return result;
}
} // namespace entis::launcher
