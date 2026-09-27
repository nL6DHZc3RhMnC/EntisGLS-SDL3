#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_input.h"
#include "legacy_input_probe.h"
#include "platform/log.h"
#include <memory>
#include <thread>

namespace {
bool Check(bool condition,const char *stage) {
    if(!condition)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy input probe FAIL: %s",stage);
    return condition;
}
}
bool CheckLegacyInput(ECSEnvironment &environment) {
    std::unique_ptr<ESLFileObject> file(environment.OpenFileObject("input.xml"));
    if(!Check(bool(file),"open original input.xml from NOA"))return false;
    EStreamBuffer bytes;const auto length=file->GetLength();
    if(!Check(file->Read(bytes.PutBuffer(length),length)==length,"read original input.xml"))return false;
    bytes.Flush(length);EDescription xml;
    if(!Check(!xml.ReadDescription(bytes,EDescription::dftXML),"parse original input.xml"))return false;
    ECSInputFilter input;
    if(!Check(!input.LoadInputFilter(xml),"load 26 original mappings and context flags"))return false;
    if(!Check(!input.FlushInputQueue(8),"enable bounded mapped queue"))return false;
    ECSInputFilter::InputEvent event;
    if(!Check(!input.OnMouseButton(1,123,45,true) && input.IsJoyButtonPushing(4) &&
        input.GetJoyButtonPushed(4)==1,"transparent touch-down maps to button 0"))return false;
    if(!Check(!input.GetInputEvent(event) && event.device==ECSInputFilter::JoyStick && event.key==4,
        "queue contains mapped joystick event"))return false;
    input.OnMouseButton(1,123,45,false);
    if(!Check(!input.IsJoyButtonPushing(4) && input.GetJoyButtonPushed(4)==1 &&
        input.GetInputEvent(event)==eslErrTimeout,"release clears held state without enqueuing press"))return false;
    input.FlushJoyButtonPushed();input.OnKey('Z',0,true);
    if(!Check(input.GetJoyButtonPushed(4)==1 && input.IsJoyButtonPushing(4),"keyboard mapping"))return false;
    input.FlushJoyButtonPushed();
    if(!Check(input.GetJoyButtonPushed(4)==0 && input.IsJoyButtonPushing(4),"flush preserves held state"))return false;
    input.OnFocusLost();
    if(!Check(!input.IsJoyButtonPushing(4),"focus loss releases held buttons"))return false;
    input.FlushInputQueue(4);input.OnKey(13,SakuraGL::vkeyContextMenu,true);
    if(!Check(!input.GetInputEvent(event) && event.device==ECSInputFilter::Command &&
        event.command==L"ID_FULLSCREEN","SDK Alt modifier translates to legacy 0x400D"))return false;
    input.OnKey(18,0,true);input.OnKey(13,0,true);input.OnKey(13,0,false);input.OnKey(18,0,false);
    if(!Check(!input.GetInputEvent(event) && event.key==18 &&
        !input.GetInputEvent(event) && event.device==ECSInputFilter::Command && event.command==L"ID_FULLSCREEN",
        "Android zero-flags key callbacks retain Alt modifier"))return false;
    input.OnKey(13,0,true);input.OnKey(13,0,false);
    if(!Check(!input.GetInputEvent(event) && event.device==ECSInputFilter::JoyStick && event.key==4,
        "Alt key release clears modifier"))return false;
    input.OnKey(27,0,true);
    if(!Check(!input.GetInputEvent(event) && event.device==ECSInputFilter::Command &&
        event.command==L"ID_ESCAPE_KEY","escape command mapping"))return false;
    input.OnKey(38,0,true);
    SakuraGL::S4DVector position;
    if(!Check(!input.GetStickPosition(position) && position.x==0 && position.y==-1,
        "virtual directional pad produces analog position"))return false;
    input.OnKey(38,0,false);input.FlushInputQueue(1);
    input.OnKey('Z',0,true);input.OnKey('X',0,true);
    if(!Check(!input.GetInputEvent(event) && event.key==5 && input.GetInputEvent(event)==eslErrTimeout,
        "queue overflow discards oldest event"))return false;
    double x,y;input.GetCursorPosition(x,y);
    if(!Check(x==123&&y==45,"logical client cursor position"))return false;

    // Save/load uses the original InputFilter stream shape, including 32-bit
    // mode/XML-length/queue-limit. No live event queue is serialized.
    ECSContext context;EMemoryFile saved;saved.Create(64);
    if(!Check(!input.Save(saved,context),"save filter metadata"))return false;
    saved.Seek(0,ESLFileObject::FromBegin);ECSInputFilter restored;
    if(!Check(!restored.Load(saved,context),"restore filter metadata"))return false;
    restored.OnKey('Z',0,true);
    if(!Check(!restored.GetInputEvent(event) && event.key==4 && restored.IsJoyButtonPushing(4),
        "restored mapping and queue limit"))return false;

    input.FlushInputQueue(4);
    std::thread producer([&input](){Sleep(20);input.OnMouseButton(1,8,9,true);});
    const auto waitResult=input.GetInputEvent(event,1000);
    producer.join();
    if(!Check(!waitResult&&event.device==ECSInputFilter::JoyStick&&event.key==4,"event wait wakes on real queue signal"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy input probe PASS: original XML, touch/key mapping, mapped queue, Alt command, virtual pad, focus, save/load, wait");
    return true;
}
