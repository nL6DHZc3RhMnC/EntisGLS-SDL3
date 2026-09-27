// Standalone integration fixture linked against the built SDL/legacy libraries.
#include "legacy_compat/gls.h"
#include "legacy_window.h"
#include "platform/sdl/system.h"
#include "platform/sdl/window.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>

SakuraGL::SGLError sglStaticInitialize() { return SakuraGL::sglErrSuccess; }
SakuraGL::SGLError sglStaticFinalize() { return SakuraGL::sglErrSuccess; }

namespace {
void Require(bool success, const char* text) {
    if (!success) throw std::runtime_error(text);
}
class EmptyImage final : public ECSExecutionImage {
public:
    EmptyImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1; m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096; m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX; m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
void RunFixture() {
    ECSEnvironment environment;
    EmptyImage image; image.AttachCSEnvironment(&environment);
    ECSContext context;
    Require(!context.InitializeContext(&image),"initialize context");
    struct ReleaseContext {ECSContext& context; ~ReleaseContext(){context.ReleaseContext(true);}} release{context};
    ECSWindow window;
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&window)); args.Add(new ECSString(L"SDL cursor cache probe"));
    args.Add(new ECSInteger(SakuraGL::Window::modeWindow));
    args.Add(new ECSInteger(160)); args.Add(new ECSInteger(90)); args.Add(new ECSInteger(32));
    int index=0;
    Require(!window.GetFunction(context,index,L"CreateDisplay"),"find CreateDisplay");
    Require(!window.CallFunction(context,index,args),"CreateDisplay invocation");
    std::unique_ptr<ECSObject> returned(context.PopObject());
    INT64 status=1;
    Require(returned && !returned->OperateInteger(status) && !status,"create actual SDK/SDL window");
    auto* host=SDL_GL_GetCurrentWindow();
    auto* native=window.NativeWindow();
    Require(host && native,"current native window");
    SDL_HideWindow(host);
    SDL_WarpMouseInWindow(host,5,5);
    float beforeX=0,beforeY=0;
    SDL_GetMouseState(&beforeX,&beforeY);
    SDL_Event event{};
    event.type=SDL_EVENT_MOUSE_MOTION; event.motion.windowID=SDL_GetWindowID(host);
    event.motion.x=37.25f; event.motion.y=25.25f;
    native->OnSDLEvent(event);
    float afterX=0,afterY=0;
    SDL_GetMouseState(&afterX,&afterY);
    Require(beforeX==afterX && beforeY==afterY,"received input must not warp SDL mouse position");
    SakuraGL::SGLPoint cursor;
    native->GetCursorPosition(cursor,0);
    Require(cursor.x==37 && cursor.y==25,"received input updates SDK logical cursor cache");
    Require(!window.MoveLogicalCursor(47,31),"explicit logical cursor movement");
    native->GetCursorPosition(cursor,0);
    SDL_GetMouseState(&afterX,&afterY);
    Require(cursor.x==47 && cursor.y==31,"logical move updates SDK cursor cache");
    Require(beforeX==afterX && beforeY==afterY,"logical input refresh must not warp SDL mouse");
    Require(!native->MoveCursorPosition(61,43,0),"explicit physical cursor move");
    SDL_GetMouseState(&afterX,&afterY);
    Require(std::abs(afterX-61)<.01f && std::abs(afterY-43)<.01f,
            "explicit SDK cursor move still warps SDL mouse");
    native->GetCursorPosition(cursor,0);
    Require(cursor.x==61 && cursor.y==43,"explicit physical move preserves logical cache");
    std::printf("PASS received input and logical moves update SDK cache without warp; explicit SDK move warps SDL mouse\n");
    window.CloseDisplay();
}
} // namespace

int main() {
    std::setvbuf(stdout,nullptr,_IOLBF,0);
    int result=0;
    if(!SDL_Init(SDL_INIT_VIDEO)) return 1;
    study::platform::sdl::SystemPaths paths;
    paths.localRoot=(std::filesystem::temp_directory_path()/"studysteady-cursor-probe").string();
    paths.storageRoot=paths.localRoot;
    Require(study::platform::sdl::ConfigureSystemPaths(paths),"configure fixture paths");
    SakuraGL::Initialize(); ECotophaScript::Initialize(0); ECotophaScript::MultithreadReference(true);
    try { RunFixture(); }
    catch(const std::exception& error) {std::fprintf(stderr,"FAIL %s\n",error.what());result=1;}
    ECotophaScript::Release(); SakuraGL::Finalize();
    study::platform::sdl::CollectRetiredWindows(); SDL_Quit();
    return result;
}
