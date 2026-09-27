#include "extensions/emote/win_atlas.h"
#include "PSBPackedInternal.h"
#include "motion_math.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>

namespace {
using Node = PSB::PSBRawNode;
using Affine = studysteady::motion::math::Affine2x3;
std::vector<Node> array(const Node &node) {
    if(node.GetTypeCategory() != 6) throw std::runtime_error("Expected PSB array");
    const auto *data = node.GetNode();
    const PSB::detail::PsbArray_guess offsets(data + 1);
    if(offsets.nElementCount > 2000000) throw std::runtime_error("PSB array too large");
    std::vector<Node> result;
    for(uint32_t i = 0; i < offsets.nElementCount; ++i)
        result.emplace_back(node.GetFile_guess(), data + 1 + offsets.nBytes + offsets[i]);
    return result;
}
double number(const Node &node, const char *key, double fallback = 0) {
    Node value; return node.GetDictionaryValue(key, value) && value.GetTypeCategory() != 0 ? value.GetDouble() : fallback;
}
std::string string(const Node &node, const char *key, std::string fallback = {}) {
    Node value; return node.GetDictionaryValue(key, value) && value.GetTypeCategory() == 4 ? value.GetString() : fallback;
}
bool present(const Node &node, const char *key) {
    Node value; return node.GetDictionaryValue(key, value) && value.GetTypeCategory() != 0;
}
std::string quote(const std::string &text) {
    std::ostringstream out; out << '"';
    for(unsigned char c : text) {
        if(c == '"' || c == '\\') out << '\\' << c;
        else if(c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c) << std::dec;
        else out << c;
    }
    out << '"'; return out.str();
}

struct State {
    Affine transform{1,0,0,1,0,0};
    double z = 0;
    unsigned opacity = 255;
    bool reliable = true;
    std::string path;
};
struct Selection {
    Node content;
    double time = 0, frameTime = 0;
    int index = 0, type = 0;
    bool exact = true;
    std::string parameter;
};
struct Auditor {
    Node root;
    std::map<std::string, double> variables;
    std::vector<std::string> rows, blockers;
    std::map<int, size_t> types;
    std::set<std::string> motions;
    size_t parameterCount = 0, meshCount = 0, stencilCount = 0, quadCount = 0, combinatorCount = 0;
    std::map<std::string, std::unique_ptr<studysteady::motion::WinAtlas>> atlases;

    explicit Auditor(Node root_) : root(root_) {}
    void block(const std::string &path, const std::string &why) {
        blockers.push_back("{\"path\":" + quote(path) + ",\"reason\":" + quote(why) + "}");
    }
    Selection select(const Node &layer, const Node &motion, double time) {
        Selection result;
        result.time = time;
        if(present(layer, "parameterize")) {
            const auto parameters = array(motion.GetDictionaryValueStrict("parameter"));
            const int index = int(number(layer, "parameterize"));
            if(index < 0 || size_t(index) >= parameters.size()) throw std::runtime_error("Parameter index out of bounds");
            const auto &raw = parameters[size_t(index)];
            result.parameter = string(raw, "id");
            studysteady::motion::math::Parameter parameter;
            parameter.rangeBegin = number(raw, "rangeBegin");
            parameter.rangeEnd = number(raw, "rangeEnd");
            parameter.division = number(raw, "division", parameter.rangeEnd - parameter.rangeBegin);
            parameter.discretization = number(raw, "discretization") != 0;
            studysteady::motion::math::normalizeParameterValue_guess(parameter, variables[result.parameter]);
            result.time = parameter.value;
            ++parameterCount;
        }
        const auto frames = array(layer.GetDictionaryValueStrict("frameList"));
        if(frames.size() < 2) throw std::runtime_error("Timeline requires two sentinel-compatible frames");
        // PlayerUpdateLayerEval.cpp::initialNodeFrameIndexForTime_guess.
        int selected = 0;
        for(size_t i = 0; i < frames.size(); ++i) {
            const double frameTime = number(frames[i], "time");
            if(result.time == frameTime) { selected = int(i); break; }
            if(result.time < frameTime) { selected = int(i) - 1; break; }
        }
        result.index = std::min(selected, int(frames.size()) - 2);
        if(result.index < 0) throw std::runtime_error("Timeline time precedes first frame");
        const auto &frame = frames[size_t(result.index)];
        result.type = int(number(frame, "type"));
        result.frameTime = number(frame, "time");
        if(result.type != 0) result.content = frame.GetDictionaryValueStrict("content");
        if(result.type == 3 && number(frames[size_t(result.index + 1)], "type") != 0)
            result.exact = std::fabs((result.time - result.frameTime) / (number(frames[size_t(result.index + 1)], "time") - result.frameTime)) < 1e-7;
        return result;
    }
    void visitMotion(const std::string &chara, const std::string &name, State parent, double time = 0, unsigned depth = 0) {
        if(depth > 64) throw std::runtime_error("Child-motion recursion limit exceeded");
        const auto key = chara + "/" + name;
        const auto motion = root.GetDictionaryValueStrict("object").GetDictionaryValueStrict(chara.c_str()).GetDictionaryValueStrict("motion").GetDictionaryValueStrict(name.c_str());
        motions.insert(key);
        parent.path += "/motion:" + key;
        for(const auto &layer : array(motion.GetDictionaryValueStrict("layer"))) visitLayer(layer, motion, parent, time, depth);
    }
    void visitLayer(const Node &layer, const Node &motion, State parent, double time, unsigned depth) {
        if(rows.size() > 10000) throw std::runtime_error("Node limit exceeded");
        State state = parent;
        state.path += "/" + string(layer, "label");
        const int type = int(number(layer, "type"));
        ++types[type];
        const auto selected = select(layer, motion, time);
        if(selected.type == 0) return;
        if(!selected.exact) { block(state.path, "nonzero crossfade requires the full two-slot interpolation core"); state.reliable = false; }
        if(type != 0 && type != 1 && type != 2 && type != 3 && type != 12) {
            block(state.path, "unsupported node type " + std::to_string(type)); state.reliable = false;
        }
        const Node &content = selected.content;
        const uint32_t mask = static_cast<uint32_t>(number(content, "mask"));
        const uint32_t inherit = static_cast<uint32_t>(number(layer, "inheritMask"));
        if(number(layer, "coordinate") != 0 || number(layer, "groundCorrection") != 0) {
            block(state.path, "non-2D or ground-corrected transform"); state.reliable = false;
        }
        if((inherit & 0x1fc) != 0x1fc && type != 1) {
            block(state.path, "partial transform inheritance"); state.reliable = false;
        }
        double x = 0, y = 0, z = 0;
        if(mask & 2) {
            const auto coord = array(content.GetDictionaryValueStrict("coord"));
            if(coord.size() != 3) throw std::runtime_error("Invalid coord");
            x = coord[0].GetDouble(); y = coord[1].GetDouble(); z = coord[2].GetDouble();
        }
        state.transform[4] = parent.transform[0]*x + parent.transform[2]*y + parent.transform[4];
        state.transform[5] = parent.transform[1]*x + parent.transform[3]*y + parent.transform[5];
        state.z += z;
        const auto orderArray = array(layer.GetDictionaryValueStrict("transformOrder"));
        if(orderArray.size() != 4) throw std::runtime_error("Invalid transform order");
        int order[4];
        for(size_t i = 0; i < 4; ++i) { order[i] = orderArray[i].GetInt(); if(order[i]<0 || order[i]>3) throw std::runtime_error("Unknown transform operation"); }
        studysteady::motion::math::applyLocalTransform(state.transform,
            (mask & 12) && number(content,"fx"), (mask & 12) && number(content,"fy"),
            mask & 16 ? number(content,"angle") : 0,
            mask & 96 ? number(content,"zx") : 1, mask & 96 ? number(content,"zy") : 1,
            mask & 384 ? number(content,"sx") : 0, mask & 384 ? number(content,"sy") : 0, order);
        const unsigned localOpacity = mask & 1024 ? unsigned(number(content,"opa")) : 255;
        state.opacity = (inherit & 1024) ? localOpacity * parent.opacity / 255 : localOpacity;
        if(present(layer, "meshCombinator")) {
            ++combinatorCount;
            block(state.path,"meshCombinator/rawMeshList is not consumed by the imported MotionPlayer");
            state.reliable = false;
        }
        bool hasPatch = false;
        if(present(content, "mesh")) {
            const auto mesh = content.GetDictionaryValueStrict("mesh");
            hasPatch = present(mesh,"bp") || present(mesh,"bezierPatch");
        }
        if(hasPatch) { ++meshCount; block(state.path,"live Bezier patch requires mesh tessellation and ancestor mapping"); state.reliable = false; }
        if(type == 12 || number(layer,"stencilType") != 0) {
            ++stencilCount; block(state.path,"composite stencil requires named mask links and render pass ordering");
        }
        if(mask & (0x40000u | 0x200u)) block(state.path,"action or packed color weight requires runtime handling");
        std::ostringstream row; row << std::setprecision(17);
        row << "{\"path\":" << quote(state.path) << ",\"type\":" << type << ",\"frame\":" << selected.index
            << ",\"sampleTime\":" << selected.time << ",\"parameter\":" << quote(selected.parameter)
            << ",\"affineReliable\":" << (state.reliable ? "true" : "false") << ",\"affine\":[";
        for(size_t i=0;i<6;++i) { if(i) row << ','; row << state.transform[i]; }
        row << "],\"z\":" << state.z << ",\"opacity\":" << state.opacity;
        const std::string src = string(content,"src"), icon = string(content,"icon");
        if(type == 0 && src != "blank" && !src.empty()) {
            auto &atlas = atlases[src];
            if(!atlas) atlas = std::make_unique<studysteady::motion::WinAtlas>(root,src);
            const auto &source = atlas->icon(icon);
            ++quadCount;
            row << ",\"source\":" << quote(src) << ",\"icon\":" << quote(icon)
                << ",\"sourceRect\":[" << source.left << ',' << source.top << ',' << source.width << ',' << source.height << ']';
            if(state.reliable) {
                const double ox = source.originX + ((mask & 1) ? number(content,"ox") : 0);
                const double oy = source.originY + ((mask & 1) ? number(content,"oy") : 0);
                const double positions[8] = {-ox,-oy,source.width-ox,-oy,source.width-ox,source.height-oy,-ox,source.height-oy};
                row << ",\"affineQuad\":[";
                for(size_t i=0;i<4;++i) {
                    if(i) row << ',';
                    row << state.transform[0]*positions[i*2]+state.transform[2]*positions[i*2+1]+state.transform[4] << ','
                        << state.transform[1]*positions[i*2]+state.transform[3]*positions[i*2+1]+state.transform[5];
                }
                row << ']';
            }
        }
        row << '}'; rows.push_back(row.str());
        if(type == 3 && !src.empty()) {
            if(src.find('/') != std::string::npos) throw std::runtime_error("External child motion path unsupported");
            double offset = 0;
            if(present(content,"motion")) {
                auto info = content.GetDictionaryValueStrict("motion");
                if(number(info,"mask") != 0) block(state.path,"child motion override flags require the full child Player bridge");
                offset = number(info,"timeOffset");
            }
            visitMotion(src,icon,state,std::max(0.0,time-selected.frameTime+offset),depth+1);
        }
        // The passthrough flag makes descendants resolve to the first ordinary
        // ancestor; current type-12 nodes must not contribute a new transform.
        const auto &childParent = inherit & 0x400000 ? parent : state;
        for(const auto &child : array(layer.GetDictionaryValueStrict("children"))) visitLayer(child,motion,childParent,time,depth);
    }
    void write(std::ostream &out) {
        out << "{\"status\":\"incomplete\",\"renderable\":false,\"scope\":\"base graph at frame 0; zero variables; selector option 0\",\"motionCount\":" << motions.size()
            << ",\"layerCount\":" << rows.size() << ",\"parameterizedLayerCount\":" << parameterCount << ",\"liveBezierPatchCount\":" << meshCount
            << ",\"meshCombinatorCount\":" << combinatorCount << ",\"stencilCount\":" << stencilCount << ",\"candidateQuadCount\":" << quadCount << ",\"nodes\":[";
        for(size_t i=0;i<rows.size();++i) { if(i) out << ','; out << rows[i]; }
        out << "],\"blockers\":[";
        for(size_t i=0;i<blockers.size();++i) { if(i) out << ','; out << blockers[i]; }
        out << "]}\n";
    }
};
}

int main(int argc,char **argv) {
    if(argc != 4) { std::cerr << "Usage: motion_pose_probe file.psb seed output.json\n"; return 2; }
    try {
        const auto file = studysteady::motion::loadPsb(argv[1],static_cast<uint32_t>(std::stoull(argv[2])));
        Auditor auditor(file.GetRoot());
        const auto metadata = auditor.root.GetDictionaryValueStrict("metadata");
        Node selectors;
        if(metadata.GetDictionaryValue("selectorControl",selectors)) for(const auto &selector:array(selectors)) {
            if(!number(selector,"enabled")) continue;
            size_t index = 0;
            for(const auto &option:array(selector.GetDictionaryValueStrict("optionList"))) {
                auditor.variables[string(option,"label")] = number(option,index++ == 0 ? "onValue" : "offValue");
            }
        }
        const auto base = metadata.GetDictionaryValueStrict("base");
        auditor.visitMotion(string(base,"chara"),string(base,"motion"),{});
        auditor.block("/", "final draw list ordering and controller physics have not been integrated");
        std::ofstream out(argv[3]); auditor.write(out);
        if(!out) throw std::runtime_error("Cannot write pose audit");
        std::cout << "motions=" << auditor.motions.size() << " layers=" << auditor.rows.size() << " parameters=" << auditor.parameterCount
            << " mesh_combinators=" << auditor.combinatorCount << " source_quads=" << auditor.quadCount << " live_patches=" << auditor.meshCount << " stencils=" << auditor.stencilCount
            << " blockers=" << auditor.blockers.size() << " renderable=false\n";
        // Distinct nonzero status: audit is complete, but it is not a frame.
        return 3;
    } catch(const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}
