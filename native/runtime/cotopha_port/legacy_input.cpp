#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_input.h"
#include "runtime/cotopha_port/legacy_window.h"
#include <sakuraglx/sprite/sglx_sprite_window.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <tuple>

IMPLEMENT_CLASS_INFO(ECSInputFilter, ECSObject)
const wchar_t *ECSInputFilter::m_pwszFuncName[19] = {
    L"LoadInputFilter", L"DeleteInputFilter", L"OpenFilter", L"CloseFilter",
    L"GetInputEvent", L"FlushInputQueue", L"GetCapturedJoyStick", L"GetStickPosition",
    L"IsJoyButtonPushing", L"GetJoyButtonPushed", L"FlushJoyButtonPushed",
    L"ResetJoyButtonPushing", L"GetCursorPos", L"MoveCursorPos", L"AddFilter",
    L"RemoveFilter", L"GetFilter", L"DispatchEvent", nullptr
};
namespace {
constexpr uint32_t Pushing = 0x80000000u, CountMask = 0x7fffffffu;
constexpr uint32_t Transparent = 0x100, ContextKey = 0x200;
const wchar_t *devices[] = {L"keyboard",L"mouse",L"joystick",L"command",L"signal"};
bool validEvent(const ECSInputFilter::InputEvent &event) {
    return event.device >= 0 && event.device <= 4 && event.number >= 0 &&
        (event.device != ECSInputFilter::JoyStick || (event.number < 6 && event.key >= 0 && event.key < 36));
}
}
bool ECSInputFilter::InputEvent::operator<(const InputEvent &other) const {
    return std::tie(device,number,key,command) < std::tie(other.device,other.number,other.key,other.command);
}
ECSInputFilter::ECSInputFilter() : ready_(CreateEvent(nullptr, TRUE, FALSE, nullptr)) { m_vtType = csvtObject; }
ECSInputFilter::~ECSInputFilter() { CloseFilter(); CloseHandle(ready_); }
const wchar_t *ECSInputFilter::GetTypeName() const { return L"InputFilter"; }
ECSObject *ECSInputFilter::GetTypeOf(const wchar_t *name) {
    return !EWideString::Compare(name, L"InputFilter") ? this : ECSObject::GetTypeOf(name);
}
ECSObject *ECSInputFilter::Duplicate() {
    auto *result = new ECSInputFilter;
    EDescription xml;
    SaveInputFilter(xml);
    result->LoadInputFilter(xml);
    return result;
}
ESLError ECSInputFilter::Move(ECSContext &context, ECSObject *object) {
    auto *source = ESLTypeCast<ECSInputFilter>(ECSObject::GetEntity(object));
    if (!source) return ESLErrorMsg("InputFilter assignment requires an InputFilter");
    if (source != this) {
        EDescription xml;
        source->SaveInputFilter(xml);
        const auto result = LoadInputFilter(xml);
        if (result) return result;
    }
    context.delete_CSObject(object);
    return eslErrSuccess;
}
ESLError ECSInputFilter::UnaryOperate(ECSContext &, CSUnaryOperatorType) { return ESLErrorMsg("InputFilter has no unary operator"); }
ESLError ECSInputFilter::Operate(ECSContext &, CSOperatorType, ECSObject *) { return ESLErrorMsg("InputFilter has no arithmetic operator"); }
ESLError ECSInputFilter::Compare(ECSContext &, int &, CSCompareType, ECSObject &) { return ESLErrorMsg("InputFilter has no value comparison"); }
ESLError ECSInputFilter::GetFunction(ECSContext &, int &index, const wchar_t *name) {
    for (index = 0; index < 18; ++index)
        if (!EWideString::Compare(name,m_pwszFuncName[index])) return eslErrSuccess;
    index = -1;
    return ESLErrorMsg("Unknown InputFilter method");
}
ESLError ECSInputFilter::ReadEvent(InputEvent &event, EDescription &tag) {
    EWideString device = tag.GetAttrString(L"device", L"");
    event.device = -1;
    for (int i=0; i<5; ++i) if (device == devices[i]) event.device = i;
    event.number = tag.GetAttrInteger(L"device_number",0);
    EStreamWideString key = tag.GetAttrString(L"key",L"");
    EWideString token = key.GetAToken();
    const wchar_t *directions[] = {L"up",L"down",L"left",L"right"};
    const int codes[] = {38,40,37,39};
    bool direction = false;
    for (int i=0;i<4;++i) if (token == directions[i]) {
        event.key = event.device == JoyStick ? i : codes[i]; direction = true;
    }
    if (!direction) {
        if (token == L"button" || token == L"code") {
            if (!key.HasToComeChar(L":")) return eslErrInvalidParam;
            event.key = key.GetInteger() + (token == L"button" ? 4 : 0);
        } else if (token == L"ascii") {
            if (!key.HasToComeChar(L":")) return eslErrInvalidParam;
            event.key = key.GetCharacter();
        } else if (event.device != Command && event.device != Signal) return eslErrInvalidParam;
    }
    event.command = tag.GetAttrString(L"command",L"");
    event.priority = tag.GetAttrInteger(L"cmd_priority",event.device == Signal ? 1 : 0);
    return validEvent(event) ? eslErrSuccess : eslErrInvalidParam;
}
void ECSInputFilter::WriteEvent(EDescription &tag, const InputEvent &event) {
    tag.SetAttrString(L"device",devices[event.device]);
    tag.SetAttrInteger(L"device_number",event.number);
    tag.SetAttrString(L"key",L"code:" + EWideString(event.key));
    if (!event.command.empty()) tag.SetAttrString(L"command",event.command.c_str());
    if (event.device == Command || event.device == Signal) tag.SetAttrInteger(L"cmd_priority",event.priority);
}
ESLError ECSInputFilter::LoadInputFilter(const wchar_t *path, ECSContext *context) {
    if (!context) return eslErrInvalidParam;
    std::unique_ptr<ESLFileObject> file(context->OpenFileOnScript(path));
    if (!file) return eslErrGeneral;
    const auto length = file->GetLength();
    if (length > 4*1024*1024) return eslErrInvalidParam;
    EStreamBuffer buffer;
    if (file->Read(buffer.PutBuffer(length),length) != length) return eslErrGeneral;
    buffer.Flush(length);
    EDescription xml;
    const auto error = xml.ReadDescription(buffer,EDescription::dftXML);
    if (error) return error;
    return LoadInputFilter(xml);
}
ESLError ECSInputFilter::LoadInputFilter(EDescription &xml) {
    auto *tag = xml.GetContentTagAs(0,L"filter");
    if (!tag) return eslErrInvalidParam;
    uint32_t flags = 0;
    EStreamWideString tokens = tag->GetAttrString(L"flags",L"normal");
    while (!tokens.DisregardSpace()) {
        const auto token = tokens.GetAToken();
        if (token == L"transparent") flags |= Transparent;
        else if (token == L"context_key") flags |= ContextKey;
        else if (token != L"normal") return eslErrNotSupported;
    }
    const int joyCount = tag->GetAttrInteger(L"joystick_count",6);
    const double threshold = tag->GetAttrReal(L"joystick_threshold",0.01);
    if (joyCount < 0 || joyCount > 6 || !std::isfinite(threshold) || threshold < 0 || threshold > 1)
        return eslErrInvalidParam;
    std::map<InputEvent,InputEvent> parsed;
    for (int i=0;i<tag->GetContentTagCount();++i) {
        auto *mapping = tag->GetContentTagAt(i);
        if (!mapping || mapping->Tag() != L"key_assign") continue;
        auto *input = mapping->GetContentTagAs(0,L"input");
        auto *output = mapping->GetContentTagAs(0,L"output");
        if (!input || !output) return eslErrInvalidParam;
        InputEvent in,out;
        auto error = ReadEvent(in,*input); if (error) return error;
        error = ReadEvent(out,*output); if (error) return error;
        parsed[in] = out;
    }
    CloseFilter();
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    filters_ = std::move(parsed); flags_ = flags; joyCount_ = joyCount; threshold_ = threshold;
    for (auto &pad:buttons_) pad.fill(0);
    return eslErrSuccess;
}
ESLError ECSInputFilter::SaveInputFilter(EDescription &xml) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    auto *tag = new EDescription;
    tag->SetTag(L"filter"); xml.AddContentTag(tag);
    tag->SetAttrString(L"flags", (flags_ & ContextKey) ? ((flags_ & Transparent) ? L"transparent context_key" : L"context_key") : ((flags_ & Transparent) ? L"transparent" : L"normal"));
    tag->SetAttrInteger(L"joystick_count",joyCount_);
    tag->SetAttrReal(L"joystick_threshold",threshold_);
    for (const auto &pair:filters_) {
        auto *mapping = new EDescription; mapping->SetTag(L"key_assign");tag->AddContentTag(mapping);
        auto *in = new EDescription; in->SetTag(L"input");mapping->AddContentTag(in);WriteEvent(*in,pair.first);
        auto *out = new EDescription; out->SetTag(L"output");mapping->AddContentTag(out);WriteEvent(*out,pair.second);
    }
    return eslErrSuccess;
}
void ECSInputFilter::DeleteInputFilter() { CloseFilter(); std::lock_guard<std::recursive_mutex> guard(mutex_);filters_.clear(); }
ESLError ECSInputFilter::OpenFilter(ECSWindow *window,int mode,ECSContext *context) {
    if (!window) return eslErrInvalidParam;
    CloseFilter();
    { std::lock_guard<std::recursive_mutex> guard(mutex_);
      window_ = window; mode_ = mode == 0 ? 1 : 2; windowReference_.SetReference(window,context); }
    window->AttachInputFilter(this,mode == 0 ? 0 : 1);
    OnWindowReady(window);
    return eslErrSuccess;
}
void ECSInputFilter::CloseFilter() {
    ECSWindow *window;
    { std::lock_guard<std::recursive_mutex> guard(mutex_);
      window = window_;
      if (capturing_) { joystick_.ReleaseCapture(window ? window->GetWindow() : nullptr); capturing_ = false; }
      window_ = nullptr;mode_ = 0;captured_ = 0;physicalButtons_.fill(0);
      windowReference_.SetReference(nullptr,nullptr); }
    if (window) window->DetachInputFilter(this);
}
void ECSInputFilter::OnWindowReady(ECSWindow *window) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (window != window_ || capturing_ || !window->GetWindow()) return;
    const auto result = joystick_.BeginCapture(window->GetWindow());
    capturing_ = result == SakuraGL::sglErrSuccess;
    if (capturing_) PollJoyStick();
}
void ECSInputFilter::OnWindowDestroyed(ECSWindow *window) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (window != window_) return;
    if (capturing_) joystick_.ReleaseCapture(window->GetWindow());
    capturing_ = false; captured_ = 0;window_ = nullptr;mode_ = 0;
    physicalButtons_.fill(0); windowReference_.SetReference(nullptr,nullptr); OnFocusLost();
}
void ECSInputFilter::OnFocusLost() {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    for (auto &pad:buttons_) for (auto &button:pad) button &= CountMask;
    modifierFlags_ = 0; capsHeld_ = false;
}
ESLError ECSInputFilter::AddFilter(const InputEvent &in,const InputEvent &out) {
    if (!validEvent(in) || !validEvent(out)) return eslErrInvalidParam;
    std::lock_guard<std::recursive_mutex> guard(mutex_);filters_[in]=out;return eslErrSuccess;
}
ESLError ECSInputFilter::RemoveFilter(const InputEvent &in) {
    std::lock_guard<std::recursive_mutex> guard(mutex_); filters_.erase(in);return eslErrSuccess;
}
bool ECSInputFilter::GetFilter(InputEvent &out,const InputEvent &in) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    auto found=filters_.find(in);out = found==filters_.end()?in:found->second;return found!=filters_.end();
}
bool ECSInputFilter::ProcessEvent(const InputEvent &input,bool down) {
    if (!validEvent(input)) return false;
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    InputEvent output = input;
    auto found = filters_.find(input);
    const bool mapped = found != filters_.end();
    if (mapped) {
        output = found->second;
        for (int i=0;i<16;++i) { found=filters_.find(output);if(found==filters_.end())break;output=found->second; }
    }
    if (down && limit_ > 0) {
        if (queue_.size() >= static_cast<size_t>(limit_)) queue_.pop_front();
        queue_.push_back(output);SetEvent(ready_);
    }
    if (output.device == JoyStick && output.number < joyCount_) {
        auto &state = buttons_[output.number][output.key];
        state = down ? Pushing | ((state+1)&CountMask) : state&CountMask;
    }
    if (down && window_ && (output.device == Command || output.device == Signal))
        window_->QueueInputCommand(output.command.c_str(),output.priority,output.device==Signal);
    return mapped;
}
bool ECSInputFilter::Consume(bool mapped) { std::lock_guard<std::recursive_mutex> guard(mutex_);return mapped && !(flags_&Transparent); }
bool ECSInputFilter::OnKey(int64_t key,int64_t flags,bool down) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    InputEvent event;event.key=key&0xff;
    // The supplied SDK's Android KeyDown/Up dispatches flags=0. Track modifier
    // events here so legacy context_key mappings still see real Alt/Ctrl/Shift.
    int64_t modifier=0;
    if(event.key==16)modifier=SakuraGL::vkeyContextShift;
    else if(event.key==17)modifier=SakuraGL::vkeyContextControl;
    else if(event.key==18)modifier=SakuraGL::vkeyContextMenu;
    if(modifier) { if(down)modifierFlags_|=modifier;else modifierFlags_&=~modifier; }
    if(event.key==20) { if(down&&!capsHeld_)modifierFlags_^=SakuraGL::vkeyContextCapital;capsHeld_=down; }
    flags |= modifierFlags_;
    if ((flags_&ContextKey) && event.key!=16 && event.key!=17 && event.key!=18 && event.key!=20) {
        if(flags&SakuraGL::vkeyContextShift) event.key|=0x1000;
        if(flags&SakuraGL::vkeyContextControl) event.key|=0x2000;
        if(flags&SakuraGL::vkeyContextMenu) event.key|=0x4000;
        if(flags&SakuraGL::vkeyContextCapital) event.key|=0x8000;
    }
    return Consume(ProcessEvent(event,down));
}
bool ECSInputFilter::OnMouseButton(int code,double x,double y,bool down) {
    OnMouseMove(x,y);InputEvent event;event.device=Mouse;event.key=code;
    return Consume(ProcessEvent(event,down));
}
void ECSInputFilter::OnMouseMove(double x,double y) { std::lock_guard<std::recursive_mutex> guard(mutex_);cursorX_=x;cursorY_=y; }
bool ECSInputFilter::OnMouseWheel(int32_t delta,double x,double y) {
    OnMouseMove(x,y);if(!delta)return false;
    InputEvent event;event.device=Mouse;event.key=delta<0?38:40;
    ProcessEvent(event,true);ProcessEvent(event,false);return Consume(true);
}
void ECSInputFilter::GetCursorPosition(double &x,double &y) { std::lock_guard<std::recursive_mutex> guard(mutex_);x=cursorX_;y=cursorY_; }
ESLError ECSInputFilter::MoveCursorPosition(int x,int y) {
    // UI dispatch takes the same global lock before the filter mutex. Keep the
    // attached Window alive within that graphics lifecycle while moving it.
    SSystem::Lock();struct Unlock {~Unlock(){SSystem::Unlock();}} unlock;
    ECSWindow* window;double previousX,previousY;
    {std::lock_guard<std::recursive_mutex> guard(mutex_);window=window_;
     if(!window)return ESLErrorMsg("InputFilter.MoveCursorPos requires an attached Window");
     previousX=cursorX_;previousY=cursorY_;cursorX_=x;cursorY_=y;}
    const auto error=window->MoveLogicalCursor(x,y);
    if(error){std::lock_guard<std::recursive_mutex> guard(mutex_);cursorX_=previousX;cursorY_=previousY;}
    return error;
}
void ECSInputFilter::PollJoyStick() {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (!capturing_) return;
    captured_=0;
    for (int device=0;device<6;++device) {
        SakuraGL::UI::SGLJoyStickState state;
        const bool available = joystick_.PollJoyStick(state,device)==SakuraGL::sglErrSuccess;
        if (available) captured_|=1u<<device;
        uint64_t pressed=available?state.stateButtons:0;
        if (available) {
            if(state.vStickPos.y < -threshold_)pressed|=1u<<0;
            if(state.vStickPos.y > threshold_)pressed|=1u<<1;
            if(state.vStickPos.x < -threshold_)pressed|=1u<<2;
            if(state.vStickPos.x > threshold_)pressed|=1u<<3;
        }
        physicalState_[device]=state;
        const auto changed=pressed^physicalButtons_[device];physicalButtons_[device]=pressed;
        for(int key=0;key<36;++key) if(changed&(uint64_t(1)<<key)) {
            InputEvent event;event.device=JoyStick;event.number=device;event.key=key;
            ProcessEvent(event,(pressed&(uint64_t(1)<<key))!=0);
        }
    }
}
uint32_t ECSInputFilter::GetCapturedJoyStick() { std::lock_guard<std::recursive_mutex> guard(mutex_);PollJoyStick();return captured_; }
bool ECSInputFilter::IsJoyButtonPushing(int key,int device) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);PollJoyStick();
    return device>=0&&device<joyCount_&&key>=0&&key<36&&(buttons_[device][key]&Pushing);
}
int ECSInputFilter::GetJoyButtonPushed(int key,int device) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);PollJoyStick();
    return device>=0&&device<joyCount_&&key>=0&&key<36 ? buttons_[device][key]&CountMask : 0;
}
ESLError ECSInputFilter::FlushJoyButtonPushed(int device,int key) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);if(device<0||device>=joyCount_)return eslErrGeneral;
    if(key<0||key>=36) {for(auto &state:buttons_[device])state&=Pushing;}else buttons_[device][key]&=Pushing;
    return eslErrSuccess;
}
ESLError ECSInputFilter::ResetJoyButtonPushing(int device,int key) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);if(device<0||device>=joyCount_)return eslErrGeneral;
    if(key<0||key>=36) {for(auto &state:buttons_[device])state&=CountMask;}else buttons_[device][key]&=CountMask;
    return eslErrSuccess;
}
ESLError ECSInputFilter::GetStickPosition(SakuraGL::S4DVector &position,int device) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);PollJoyStick();position=SakuraGL::S4DVector(0,0,0,0);
    if(device<0||device>=joyCount_)return eslErrGeneral;
    if(captured_&(1u<<device))position=physicalState_[device].vStickPos;
    // Android exposes a single aggregate gamepad slot even without a physical
    // controller. Retain mapped keyboard/touch directions when its axes are idle.
    if(std::abs(position.x)<=threshold_)position.x=((buttons_[device][3]&Pushing)?1:0)-((buttons_[device][2]&Pushing)?1:0);
    if(std::abs(position.y)<=threshold_)position.y=((buttons_[device][1]&Pushing)?1:0)-((buttons_[device][0]&Pushing)?1:0);
    return eslErrSuccess;
}
ESLError ECSInputFilter::FlushInputQueue(int limit) {
    if(limit<0||limit>1024*1024)return eslErrInvalidParam;
    std::lock_guard<std::recursive_mutex> guard(mutex_);limit_=limit;queue_.clear();ResetEvent(ready_);return eslErrSuccess;
}
ESLError ECSInputFilter::GetInputEvent(InputEvent &event,uint32_t timeout,ECSContext *context) {
    const auto begin=timeGetTime();
    for (;;) {
        { std::lock_guard<std::recursive_mutex> guard(mutex_);PollJoyStick();
          if(!queue_.empty()) {event=queue_.front();queue_.pop_front();if(queue_.empty())ResetEvent(ready_);return eslErrSuccess;} }
        const auto elapsed=static_cast<uint32_t>(timeGetTime()-begin);
        if(timeout!=INFINITE && elapsed>=timeout)return eslErrTimeout;
        const auto slice=timeout==INFINITE?10u:std::min(10u,timeout-elapsed);
        if(context) { const auto error=context->WaitUntilEvent(ready_,slice);if(error&&error!=eslErrTimeout)return error; }
        else if(WaitForSingleObject(ready_,slice)==WAIT_FAILED)return eslErrGeneral;
    }
}
ECSInputFilter::InputEvent ECSInputFilter::FromStructure(ECSStructure &value) {
    InputEvent event;
    event.device=value.GetMemberAsInt(L"idType",Keyboard);event.number=value.GetMemberAsInt(L"iDevNum",0);
    event.key=value.GetMemberAsInt(L"iKeyNum",0);event.command=value.GetMemberAsStr(L"strCommand",L"");
    event.priority=event.device==Signal?1:0;return event;
}
void ECSInputFilter::ToStructure(ECSStructure &value,const InputEvent &event) {
    value.SetMemberAsInt(L"idType",event.device);value.SetMemberAsInt(L"iDevNum",event.number);
    value.SetMemberAsInt(L"iKeyNum",event.key);value.SetMemberAsStr(L"strCommand",event.command.c_str());
}
ESLError ECSInputFilter::CallFunction(ECSContext &context,int index,ECSObjArray<ECSObject> &args) {
    if(index<0||index>=18)return ESLErrorMsg("Unknown InputFilter method");
    static const int minArgs[]={2,1,2,1,3,2,1,2,2,2,1,1,1,3,3,2,2,3};
    static const int maxArgs[]={2,1,3,1,3,2,1,3,3,3,3,3,1,3,3,2,2,3};
    auto error=context.VerifyArgumentCount(args,minArgs[index],maxArgs[index]);if(error)return error;
    const auto push=[&](int64_t result)->ESLError{return context.PushObject(new ECSInteger(result));};
    if(index==0) {
        ECSWideString path;error=context.GetArgumentAsStr(path,args,1,nullptr);if(error)return error;
        return push(LoadInputFilter(path,&context));
    }
    if(index==1){DeleteInputFilter();return push(0);}
    if(index==2) {
        int mode;error=context.GetArgumentAsInt(mode,args,1,0);if(error)return error;
        auto *window=ESLTypeCast<ECSWindow>(context.GetArgumentObjectAs(args,2,L"Window"));
        if(!window&&context.m_pcsxi)window=ESLTypeCast<ECSWindow>(ECSObject::GetEntity(context.m_pcsxi->GetGlobalObject(L"screen")));
        if(!window)return ESLErrorMsg("InputFilter.OpenFilter requires a Window");
        return push(OpenFilter(window,mode,&context));
    }
    if(index==3){CloseFilter();return push(0);}
    if(index==4) {
        auto *value=ESLTypeCast<ECSStructure>(ECSObject::GetEntity(args.GetAt(1)));
        if(!value)return ESLErrorMsg("InputFilter.GetInputEvent requires an InputEvent reference");
        int timeout;error=context.GetArgumentAsInt(timeout,args,2,-1);if(error)return error;
        InputEvent event;error=GetInputEvent(event,static_cast<uint32_t>(timeout),&context);
        if(!error)ToStructure(*value,event);return push(error);
    }
    if(index==5){int limit;error=context.GetArgumentAsInt(limit,args,1,0);return error?error:push(FlushInputQueue(limit));}
    if(index==6)return push(GetCapturedJoyStick());
    if(index==7) {
        auto *value=ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(args.GetAt(1)));
        if(!value)return ESLErrorMsg("InputFilter.GetStickPosition requires a vector reference");
        int device;error=context.GetArgumentAsInt(device,args,2,0);if(error)return error;
        SakuraGL::S4DVector position;error=GetStickPosition(position,device);
        if(!error){value->SetMemberAsReal(L"x",position.x);value->SetMemberAsReal(L"y",position.y);value->SetMemberAsReal(L"z",position.z);}
        return push(error);
    }
    if(index==8||index==9) {
        int key,device;error=context.GetArgumentAsInt(key,args,1,0);if(error)return error;
        error=context.GetArgumentAsInt(device,args,2,0);if(error)return error;
        return push(index==8?(IsJoyButtonPushing(key,device)?-1:0):GetJoyButtonPushed(key,device));
    }
    if(index==10||index==11) {
        int device,key;error=context.GetArgumentAsInt(device,args,1,0);if(error)return error;
        error=context.GetArgumentAsInt(key,args,2,-1);if(error)return error;
        return push(index==10?FlushJoyButtonPushed(device,key):ResetJoyButtonPushing(device,key));
    }
    if(index==12) {
        auto *point=context.CreateUserStructureObject(L"Point");if(!point)return eslErrGeneral;
        double x,y;GetCursorPosition(x,y);point->SetMemberAsInt(L"x",static_cast<int64_t>(x));point->SetMemberAsInt(L"y",static_cast<int64_t>(y));
        return context.PushObject(point);
    }
    if(index==13) {
        int x,y;error=context.GetArgumentAsInt(x,args,1,0);if(error)return error;
        error=context.GetArgumentAsInt(y,args,2,0);if(error)return error;
        return push(MoveCursorPosition(x,y));
    }
    auto *input=ESLTypeCast<ECSStructure>(ECSObject::GetEntity(args.GetAt(1)));
    if(!input)return ESLErrorMsg("InputFilter requires an InputEvent reference");
    const auto in=FromStructure(*input);
    if(!validEvent(in))return eslErrInvalidParam;
    if(index==14) {
        auto *output=ESLTypeCast<ECSStructure>(ECSObject::GetEntity(args.GetAt(2)));
        if(!output)return ESLErrorMsg("InputFilter.AddFilter requires an output InputEvent reference");
        return push(AddFilter(in,FromStructure(*output)));
    }
    if(index==15)return push(RemoveFilter(in));
    if(index==16) {
        InputEvent out;GetFilter(out,in);
        auto *value=context.CreateUserStructureObject(L"InputEvent");if(!value)return eslErrGeneral;
        ToStructure(*value,out);return context.PushObject(value);
    }
    int down;error=context.GetArgumentAsInt(down,args,2,1);if(error)return error;
    return push(ProcessEvent(in,down!=0)?-1:0);
}
void ECSInputFilter::IndexAllMember() { windowReference_.IndexAllMember(); }
void ECSInputFilter::CleanupAllReference(ECSContext &context) {
    // Keep the indexed reference for CommitAllReference; detach only the native hook.
    ECSWindow *window;
    { std::lock_guard<std::recursive_mutex> guard(mutex_);window=window_;
      if(capturing_)joystick_.ReleaseCapture(window?window->GetWindow():nullptr);
      capturing_=false;window_=nullptr;captured_=0; }
    if(window)window->DetachInputFilter(this);
    windowReference_.CleanupAllReference(context);ECSObject::CleanupAllReference(context);
}
ESLError ECSInputFilter::CommitAllReference(ECSContext &context) {
    const auto error=windowReference_.CommitAllReference(context);if(error)return error;
    auto *window=ESLTypeCast<ECSWindow>(windowReference_.m_pRef);
    const auto mode=mode_;
    return window&&mode?OpenFilter(window,(mode&2)?1:0,&context):eslErrSuccess;
}
void ECSInputFilter::OnDestruction(ECSContext &context) { CloseFilter();ECSObject::OnDestruction(context); }
ESLError ECSInputFilter::Save(ESLFileObject &file,ECSContext &context) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    auto error=windowReference_.Save(file,context);if(error)return error;
    if(file.Write(&mode_,4)!=4)return eslErrGeneral;
    EDescription xml;SaveInputFilter(xml);EMemoryFile buffer;
    error=buffer.Create(256);if(error)return error;
    error=xml.WriteDescription(buffer,0,EDescription::dftXML);if(error)return error;
    const void *bytes=buffer.GetBuffer();uint32_t length=buffer.GetLength();int32_t limit=limit_;
    if(file.Write(&length,4)!=4||file.Write(bytes,length)!=length||file.Write(&limit,4)!=4)return eslErrGeneral;
    return FlushInputQueue(limit_);
}
ESLError ECSInputFilter::Load(ESLFileObject &file,ECSContext &context) {
    CloseFilter();
    auto error=windowReference_.Load(file,context);if(error)return error;
    uint32_t mode,length;int32_t limit;
    if(file.Read(&mode,4)!=4||file.Read(&length,4)!=4||length>4*1024*1024)return eslErrGeneral;
    EStreamBuffer buffer;
    if(file.Read(buffer.PutBuffer(length),length)!=length||file.Read(&limit,4)!=4)return eslErrGeneral;
    buffer.Flush(length);EDescription xml;
    error=xml.ReadDescription(buffer,EDescription::dftXML);if(error)return error;
    // Parse through a temporary filter so the serialized reference/index path is
    // not cleared by LoadInputFilter detaching its previous window.
    ECSInputFilter decoded;error=decoded.LoadInputFilter(xml);if(error)return error;
    { std::lock_guard<std::recursive_mutex> guard(mutex_);
      filters_=std::move(decoded.filters_);flags_=decoded.flags_;joyCount_=decoded.joyCount_;threshold_=decoded.threshold_;mode_=mode; }
    return FlushInputQueue(limit);
}
