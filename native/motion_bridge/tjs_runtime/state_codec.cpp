#include "state_codec.h"
#include "EmoteEngine.h"
#include "MotionDispatch.h"
#include "tjsArray.h"
#include "tjsBinarySerializer.h"
#include "tjsDictionary.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>

namespace studysteady::motion { namespace {
constexpr size_t maximumBytes=8*1024*1024;
[[noreturn]] void invalid(const std::string &why){throw std::runtime_error("motion state: "+why);}
class MemoryStream final:public tTJSBinaryStream {
public:
    std::vector<uint8_t> bytes;size_t position=0;
    tjs_uint64 Seek(tjs_int64 offset,tjs_int whence) override {
        const int64_t base=whence==SEEK_SET?0:whence==SEEK_CUR?position:whence==SEEK_END?bytes.size():-1;
        if(base<0||offset< -base||offset>static_cast<int64_t>(bytes.size())-base)invalid("stream seek out of range");
        position=base+offset;return position;
    }
    tjs_uint Read(void *out,tjs_uint size) override {const auto n=std::min<size_t>(size,bytes.size()-position);std::memcpy(out,bytes.data()+position,n);position+=n;return n;}
    tjs_uint Write(const void *in,tjs_uint size) override {if(size>maximumBytes-position)invalid("snapshot too large");bytes.resize(std::max(bytes.size(),position+size));std::memcpy(bytes.data()+position,in,size);position+=size;return size;}
    tjs_uint64 GetSize() override{return bytes.size();}
};
std::vector<uint8_t> structuredBinary(const tTJSVariant &value){
    MemoryStream stream;stream.Write(tTJSBinarySerializer::HEADER,tTJSBinarySerializer::HEADER_LENGTH);
    std::vector<iTJSDispatch2 *> stack;tTJSArrayNI::SaveStructuredBinaryForObject(value.AsObjectNoAddRef(),stack,stream);return std::move(stream.bytes);
}
struct Node {
    enum Kind {Number,String,Array,Map} kind=Number;double number=0;std::string string;
    std::vector<Node> array;std::map<std::string,Node> map;
};
// This validator never creates TJS objects. The genuine reader is only invoked
// after every count, string, key, scalar and recursion boundary is checked.
class Reader {
    const uint8_t *bytes;size_t size,pos=8,count=0;
    uint64_t word(unsigned n){if(n>size-pos)invalid("truncated binary");uint64_t value=0;for(unsigned i=0;i<n;++i)value|=uint64_t(bytes[pos++])<<(8*i);return value;}
    Node text(size_t length){if(length>65536||length>(size-pos)/2)invalid("string length out of range");std::u16string value;value.reserve(length);
        for(size_t i=0;i<length;++i){auto c=static_cast<char16_t>(word(2));if(!c)invalid("embedded zero in string");value.push_back(c);}
        for(size_t i=0;i<value.size();++i){const auto c=value[i];if(c>=0xd800&&c<=0xdbff){if(++i==value.size()||value[i]<0xdc00||value[i]>0xdfff)invalid("invalid UTF-16");}else if(c>=0xdc00&&c<=0xdfff)invalid("invalid UTF-16");}
        Node n;n.kind=Node::String;n.string=ttstr(value.c_str()).AsStdString();return n;
    }
    Node container(bool map,size_t length,unsigned depth,const std::string &path){if(length>4096)invalid("container too large");Node n;n.kind=map?Node::Map:Node::Array;
        for(size_t i=0;i<length;++i){if(map){auto key=read(depth+1,path+"/<key>");if(key.kind!=Node::String)invalid("dictionary key is not string");auto value=read(depth+1,path+"/"+key.string);if(!n.map.emplace(key.string,std::move(value)).second)invalid("duplicate dictionary key");}else n.array.push_back(read(depth+1,path+"/"+std::to_string(i)));}return n;
    }
    Node read(unsigned depth,const std::string &path){if(depth>32||++count>50000)invalid("tree complexity out of range");const auto tag=word(1);Node n;
        if(tag<=0x7f){n.number=tag;return n;}
        if(tag>=0x80&&tag<=0x8f)return container(true,tag-0x80,depth,path);
        if(tag>=0x90&&tag<=0x9f)return container(false,tag-0x90,depth,path);
        if(tag>=0xa0&&tag<=0xbf)return text(tag-0xa0);
        switch(tag){
            case 0xc2:n.number=1;break;case 0xc3:n.number=0;break;
            case 0xc4:return text(word(1));case 0xc5:return text(word(2));case 0xc6:return text(word(4));
            case 0xca:{const uint32_t bits=word(4);float v;std::memcpy(&v,&bits,4);n.number=v;break;}
            case 0xcb:{const uint64_t bits=word(8);std::memcpy(&n.number,&bits,8);break;}
            case 0xcc:n.number=word(1);break;case 0xcd:n.number=word(2);break;case 0xce:n.number=word(4);break;
            case 0xcf:{const auto v=word(8);if(v>9007199254740991ULL)invalid("integer precision out of range");n.number=v;break;}
            case 0xd0:n.number=static_cast<int8_t>(word(1));break;case 0xd1:n.number=static_cast<int16_t>(word(2));break;
            case 0xd2:n.number=static_cast<int32_t>(word(4));break;
            case 0xd3:{const auto v=static_cast<int64_t>(word(8));if(v< -9007199254740991LL||v>9007199254740991LL)invalid("integer precision out of range");n.number=v;break;}
            case 0xdc:return container(false,word(2),depth,path);case 0xdd:return container(false,word(4),depth,path);
            case 0xde:return container(true,word(2),depth,path);case 0xdf:return container(true,word(4),depth,path);
            default:invalid("unsupported state type tag");
        }
        if(!std::isfinite(n.number)||std::fabs(n.number)>1e30)invalid("nonfinite or excessive scalar at "+path+" value="+std::to_string(n.number));return n;
    }
public:
    Reader(const void *input,size_t length):bytes(static_cast<const uint8_t *>(input)),size(length){if(!input||length<9||length>maximumBytes||std::memcmp(bytes,tTJSBinarySerializer::HEADER,8))invalid("invalid structured-binary header/size");}
    Node parse(){auto root=read(0,"state");if(pos!=size)invalid("trailing binary data");return root;}
};
const Node &field(const Node &n,const char *name){if(n.kind!=Node::Map)invalid("expected dictionary");const auto it=n.map.find(name);if(it==n.map.end())invalid(std::string("missing field ")+name);return it->second;}
double number(const Node &n){if(n.kind!=Node::Number)invalid("expected number");return n.number;}
const std::string &string(const Node &n){if(n.kind!=Node::String)invalid("expected string");return n.string;}
void keys(const Node &n,std::initializer_list<const char *> names){if(n.kind!=Node::Map||n.map.size()!=names.size())invalid("dictionary schema mismatch");for(const auto name:names)(void)field(n,name);}
void sameShape(const Node &value,const Node &schema,const std::string &path){
    if(value.kind!=schema.kind)invalid("type mismatch at "+path);
    if(value.kind==Node::Number){const auto phaseMax=path.rfind("eye/",0)==0||path.rfind("eyebrow/",0)==0?2:1;if(path.size()>=6&&path.compare(path.size()-6,6,"/phase")==0&&(value.number!=std::floor(value.number)||value.number<0||value.number>phaseMax))invalid("controller phase out of range at "+path);return;}
    if(value.kind==Node::String){if(value.string!=schema.string)invalid("label mismatch at "+path);return;}
    if(value.kind==Node::Map){if(value.map.size()!=schema.map.size())invalid("field count mismatch at "+path);for(const auto &p:schema.map){const auto it=value.map.find(p.first);if(it==value.map.end())invalid("field missing at "+path+"/"+p.first);sameShape(it->second,p.second,path+"/"+p.first);}return;}
    if(path.size()>=3&&path.compare(path.size()-3,3,"/rq")==0){for(const auto &item:value.array){keys(item,{"p0","p1"});(void)number(field(item,"p0"));(void)number(field(item,"p1"));}return;}
    if(value.array.size()!=schema.array.size())invalid("channel count mismatch at "+path);
    for(size_t i=0;i<value.array.size();++i)sameShape(value.array[i],schema.array[i],path+"/"+std::to_string(i));
}
void validateState(const Node &state,const Node &baseline,::motion::EmoteEngine &engine){
    keys(state,{"timeline","eye","eyebrow","mouth","transition","selector","base","outerforce"});
    for(const char *section:{"eye","eyebrow","mouth","transition","selector"}){
        const auto &v=field(state,section),&s=field(baseline,section);if(v.kind!=Node::Array||s.kind!=Node::Array||v.array.size()!=s.array.size())invalid(std::string("controller set mismatch: ")+section);
        std::map<std::string,const Node *> labels;for(const auto &item:s.array)labels.emplace(string(field(item,"label")),&item);
        std::set<std::string> seen;for(const auto &item:v.array){const auto &label=string(field(item,"label"));const auto it=labels.find(label);if(it==labels.end()||!seen.insert(label).second)invalid(std::string("unknown/duplicate controller label: ")+section+"/"+label);sameShape(item,*it->second,section);}
    }
    sameShape(field(state,"base"),field(baseline,"base"),"base");sameShape(field(state,"outerforce"),field(baseline,"outerforce"),"outerforce");
    // Native angle wrapping uses repeated +/- 2pi, so extreme finite input is
    // unsafe too: a float subtraction could stop making forward progress.
    const auto &angle=field(field(state,"base"),"rotate");
    for(const char *key:{"frame","prev","target"})if(std::fabs(number(field(angle,key)))>64)invalid("angle outside native normalized range");
    if(number(field(angle,"phase"))!=0&&(number(field(angle,"tick"))<0||number(field(angle,"tick"))>1||number(field(angle,"speed"))<0||number(field(angle,"exponent"))<0))invalid("invalid active angle interpolation");
    const auto &timeline=field(state,"timeline");if(timeline.kind!=Node::Array)invalid("timeline is not array");std::set<std::string> seen;
    for(const auto &item:timeline.array){keys(item,{"label","flags","curTime","blendRatioCtrl","stopWhenBlendDone"});const auto &label=string(field(item,"label"));
        const auto timelineState=engine._timelineStates.find(ttstr(label));if(timelineState==engine._timelineStates.end()||!seen.insert(label).second)invalid("unknown/duplicate timeline label");
        const auto flags=number(field(item,"flags")),time=number(field(item,"curTime"));if(flags<0||flags>4294967295.0||flags!=std::floor(flags)||time<0||time>100000000)invalid("timeline flags/time out of range");
        const auto &raw=timelineState->second.rawElement;const auto begin=::motion::detail::motionPropGetDouble(raw,TJS_W("loopBegin")),end=::motion::detail::motionPropGetDouble(raw,TJS_W("loopEnd"));
        if(begin>=0&&(!(end>begin)||time>end+0.000001))invalid("loop timeline cursor outside metadata bounds");
        (void)number(field(item,"stopWhenBlendDone"));sameShape(field(item,"blendRatioCtrl"),field(field(baseline,"base"),"scale"),"timeline/blendRatioCtrl");
    }
}
void set(tTJSVariant &object,const char *key,const tTJSVariant &value){const ttstr name(key);auto *d=object.AsObjectNoAddRef();if(TJS_FAILED(d->PropSet(TJS_MEMBERENSURE,name.c_str(),nullptr,&value,d)))invalid("dictionary write failed");}
}
std::vector<uint8_t> encodeActorState(::motion::EmoteEngine &engine,const ttstr &project,float base,float user){
    auto *dictionary=TJSCreateDictionaryObject();tTJSVariant envelope(dictionary,dictionary);dictionary->Release();
    set(envelope,"version",tTJSVariant(1));set(envelope,"project",tTJSVariant(project));set(envelope,"baseScale",tTJSVariant(base));set(envelope,"userScale",tTJSVariant(user));set(envelope,"state",engine.serializeState_guess());
    auto bytes=structuredBinary(envelope);(void)Reader(bytes.data(),bytes.size()).parse();return bytes;
}
SavedActorState decodeActorState(const void *bytes,size_t size,::motion::EmoteEngine &engine,const ttstr &project){
    const auto envelope=Reader(bytes,size).parse();keys(envelope,{"version","project","baseScale","userScale","state"});
    if(number(field(envelope,"version"))!=1||string(field(envelope,"project"))!=project.AsStdString())invalid("version/project mismatch");
    const auto base=number(field(envelope,"baseScale")),user=number(field(envelope,"userScale"));if(base!=1||std::fabs(user)>100000)invalid("wrapper scale out of range");
    const auto baselineBytes=structuredBinary(engine.serializeState_guess());const auto baseline=Reader(baselineBytes.data(),baselineBytes.size()).parse();validateState(field(envelope,"state"),baseline,engine);
    MemoryStream stream;stream.bytes.assign(static_cast<const uint8_t *>(bytes),static_cast<const uint8_t *>(bytes)+size);stream.position=8;tTJSBinarySerializer decoder;
    std::unique_ptr<tTJSVariant> decoded(decoder.Read(&stream));if(!decoded)invalid("TJS binary reader returned no object");
    return {::motion::detail::motionPropGet(*decoded,TJS_W("state")),static_cast<float>(base),static_cast<float>(user)};
}
void validateActorStateEnvelope(const void *bytes,size_t size,const ttstr &project){
    const auto envelope=Reader(bytes,size).parse();keys(envelope,{"version","project","baseScale","userScale","state"});
    if(number(field(envelope,"version"))!=1||string(field(envelope,"project"))!=project.AsStdString())invalid("version/project mismatch");
    if(number(field(envelope,"baseScale"))!=1||std::fabs(number(field(envelope,"userScale")))>100000)invalid("wrapper scale out of range");
}
}
