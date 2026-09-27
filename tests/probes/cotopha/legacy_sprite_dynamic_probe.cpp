#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_sprite_dynamic.h"
#include "runtime/cotopha_port/legacy_sprite.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include "platform/log.h"
#include <cstring>
#include <memory>

bool CheckLegacySpriteDynamic() {
    using namespace SakuraGL;
    struct Guard{Guard(){SSystem::Lock();}~Guard(){SSystem::Unlock();}} guard;
    auto check=[](bool value,const char* detail) {
        study::platform::LogPrint(value?study::platform::LogPriority::Info:study::platform::LogPriority::Error,"StudySteady",
            "Sprite dynamic probe %s: %s",value?"PASS":"FAIL",detail);return value;
    };
    ECSContext context;ECSSprite group;SGLSprite scene,child;
    if(!check(!scene.CreateBuffer(12,8)&&!group.NativeSprite().CreateBuffer(2,2)&&!child.CreateBuffer(2,1),
        "actual scene, parent cache and child buffers"))return false;
    scene.SetFillBackColor(0xff000000,true);group.NativeSprite().SetFillBackColor(0xff00ff00,true);
    child.SetFillBackColor(0xffff0000,true);
    auto place=[](SGLSprite& sprite,double x,double y) {
        auto p=sprite.GetParameter();p.vDst=S3DDVector(x,y,0);p.vCenter=S2DDVector(0,0);
        p.vZoom=S2DDVector(1,1);p.zAngle=0;p.xyCross=90;p.nTransparency=0;sprite.SetParameter(p);
        sprite.SetVisible(true);sprite.PostUpdate();
    };
    place(scene,0,0);place(group.NativeSprite(),2,2);place(child,3,1);
    group.NativeSprite().AddChild(&child);scene.AddChild(&group.NativeSprite());
    struct Detach {SGLSprite& scene;ECSSprite& group;SGLSprite& child;
        ~Detach(){scene.DetachChild(&group.NativeSprite());group.NativeSprite().DetachChild(&child);}} detach{scene,group,child};
    auto render=[&]{scene.PostUpdate();scene.PrepareDrawFrame();scene.BeforeDraw();scene.AfterDraw();scene.FinishDrawFrame();};
    auto pixel=[&](int x,int y,uint32_t expected) {
        SGLPalette value;const auto error=scene.GetFrameBuffer()->GetImage()->GetPixelRGBA(value,x,y);
        if(error||value.ui32!=expected)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
            "Sprite dynamic pixel (%d,%d) actual=%08x expected=%08x error=%d",x,y,value.ui32,expected,int(error));
        return !error&&value.ui32==expected;
    };
    auto enable=[&](ECSSprite& object,int value,INT64 expected) {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&object));args.Add(new ECSInteger(value));
        int index=-1;auto error=object.GetFunction(context,index,L"EnableDynamicMode");
        if(!error)error=object.CallFunction(context,index,args);
        INT64 old=1;ECSObject* result=error?nullptr:context.PopObject();
        if(!error)error=result?result->OperateInteger(old):eslErrGeneral;
        context.delete_CSObject(result);
        return !error&&old==expected;
    };
    render();
    if(!check(pixel(2,2,0xff00ff00)&&pixel(5,3,0xff000000),"ordinary mode draws parent background and clips external child"))return false;
    if(!check(enable(group,-1,0)&&enable(group,-1,-1),"native binding returns previous flag as 0 or -1"))return false;
    render();SGLRect rectangle;
    if(!check(pixel(2,2,0xff000000)&&pixel(5,3,0xffff0000)&&pixel(6,3,0xffff0000)&&
        group.NativeSprite().GetFrameBuffer()&&group.NativeSprite().GetRectangle(rectangle)&&
        rectangle.left==5&&rectangle.top==3&&rectangle.right==6&&rectangle.bottom==3,
        "dynamic mode skips own image, keeps buffer and renders child beyond cache bounds"))return false;
    child.SetPosition(4,1);
    if(!check(scene.HasUpdate(),"moving an external child invalidates the actual parent target"))return false;
    render();
    if(!check(pixel(5,3,0xff000000)&&pixel(6,3,0xffff0000)&&pixel(7,3,0xffff0000),
        "moved direct child erases old bounds and appears at new bounds"))return false;
    group.NativeSprite().SetTransparency(128);SGLAffine transform;
    render();SGLPalette faded;
    const auto fadeError=scene.GetFrameBuffer()->GetImage()->GetPixelRGBA(faded,2,2);
    if(!check(!GetLegacySpriteDynamicTransform(group.NativeSprite(),transform)&&!fadeError&&
        (faded.ui32&0xffff00ffu)==0xff000000u&&((faded.ui32>>8)&255)>=126&&((faded.ui32>>8)&255)<=129&&
        pixel(6,3,0xff000000),"nonzero transparency really returns to cached alpha composition"))return false;
    group.NativeSprite().SetTransparency(0);
    auto parameter=group.NativeSprite().GetParameter();parameter.vZoom=S2DDVector(2,2);group.NativeSprite().SetParameter(parameter);
    render();
    if(!check(!GetLegacySpriteDynamicTransform(group.NativeSprite(),transform)&&pixel(2,2,0xff00ff00)&&pixel(4,4,0xff00ff00)&&
        pixel(7,3,0xff000000),"nonidentity axes return to ordinary transformed cache"))return false;
    parameter.vZoom=S2DDVector(1,1);parameter.vCenter=S2DDVector(1,1);parameter.vDst=S3DDVector(3.75,3.75,0);
    group.NativeSprite().SetParameter(parameter);render();
    if(!check(GetLegacySpriteDynamicTransform(group.NativeSprite(),transform)&&transform.a13==2&&transform.a23==2&&
        pixel(6,3,0xffff0000),"rotation centre adjusts base offset and dynamic offset floors fixed coordinates"))return false;
    if(!check(enable(group,0,-1)&&enable(group,0,0),"disable restores old-state return and ordinary mode"))return false;
    render();if(!check(pixel(6,3,0xff000000),"disabling clears direct child outside the image"))return false;

    if(!check(enable(group,-1,0),"enable before original visual wire save"))return false;
    EMemoryFile saved;if(saved.Create(512))return false;
    if(!check(!SaveLegacySpriteVisual(saved,group)&&saved.GetLength()>20&&
        StudySteadyLegacyWire::Read32(static_cast<const uint8_t*>(saved.GetBuffer())+16)==1,
        "real flag occupies original Win32 visual record offset 16"))return false;
    saved.SeekLarge(0,ESLFileObject::FromBegin);ECSSprite restored;
    if(!check(!LoadLegacySpriteVisual(saved,restored),"checked original visual record reload"))return false;
    RestoreLegacySpriteVisual(restored);
    if(!check(restored.IsLegacyDynamicModeEnabled()&&enable(restored,0,-1)&&enable(restored,-1,0),
        "restoration activates actual mode and reports restored previous flag"))return false;
    if(!check(!restored.Release()&&restored.IsLegacyDynamicModeEnabled(),"Release retains the old dynamic switch"))return false;
    std::unique_ptr<ECSObject> duplicate(group.Duplicate());
    if(!check(duplicate&&!ESLTypeCast<ECSSprite>(duplicate.get())->IsLegacyDynamicModeEnabled(),
        "Duplicate follows old constructor default rather than copying dynamic server flag"))return false;
    return true;
}
