#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
#include "extensions/emote/tjs_runtime/gl_capabilities.h"
#include "ncbind.hpp"
#include "Player.h"
#include "ResourceManager.h"
#include "MotionDispatch.h"
#include "MotionBezierPatch.h"
#include "extensions/emote/tjs_runtime/psb_storage.h"
#include "EmoteEngine.h"
#include "EmoteVarController.h"
#include "extensions/emote/tjs_runtime/state_codec.h"
#include <cstdlib>
#include "extensions/emote/tjs_runtime/texture_bridge.h"
#include <cmath>
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
#include "extensions/emote/tjs_runtime/runtime_gl.h"
#include "extensions/emote/tjs_runtime/gles_render_manager.h"
#include "extensions/emote/tjs_runtime/gles_scene_bridge.h"
#include "platform/gl.h"
#endif
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <stdexcept>
#include <spdlog/sinks/stdout_color_sinks.h>

#define NCB_MODULE_NAME TJS_W("studysteady_motion_runtime.dll")
#include "MotionGeometryRegistration.inc"
using motion::Player;
using motion::ResourceManager;
NCB_REGISTER_CLASS(ResourceManager) {
    NCB_CONSTRUCTOR(());NCB_METHOD(requireLayerId);NCB_METHOD(releaseLayerId);
    NCB_METHOD(random);NCB_METHOD(findMotion);NCB_METHOD(isExistMotion);
    NCB_METHOD(findSource);
}
NCB_REGISTER_CLASS(Player) {NCB_CONSTRUCTOR((tTJSVariant));}

namespace {
std::mutex ownerMutex;
bool ownerActive=false;
std::once_flag registrarIndexOnce;
constexpr uint32_t capabilities=STUDY_MOTION_REAL_TJS|STUDY_MOTION_REAL_NCB|
    STUDY_MOTION_PSB_V4|STUDY_MOTION_PLAYER_CPU;
void copyError(char *buffer,size_t capacity,const std::string &message) {
    if(!buffer || !capacity)return;
    const auto count=std::min(capacity-1,message.size());
    std::memcpy(buffer,message.data(),count);buffer[count]=0;
}
void copyIdentifier(char *buffer,size_t capacity,const ttstr &value) {
    const auto text=value.AsStdString();
    if(!buffer || capacity<=text.size())throw std::runtime_error("identifier output buffer too small");
    std::memcpy(buffer,text.c_str(),text.size()+1);
}
}
struct StudyMotionRuntime {
    struct Project {ttstr path;tTJSVariant root;};
    struct Actor {
        uint64_t project;
        std::unique_ptr<motion::EmoteEngine> engine;
        iTVPTexture2D *target=nullptr;
        uint64_t serial=0;
        float baseScale=1,userScale=1;
        Actor(uint64_t id,const tTJSVariant &owner):project(id),engine(std::make_unique<motion::EmoteEngine>(owner)){}
        ~Actor(){if(target)target->Release();}
    };
    const std::thread::id thread=std::this_thread::get_id();
    tTJS *engine=nullptr;
    bool registered=false;
    tTJSVariant managerOwner;
    ResourceManager *manager=nullptr;
    std::unordered_map<uint64_t,Project> projects;
    uint64_t nextProject=1;
    uint64_t nextActor=1;
    std::unordered_map<uint64_t,std::unique_ptr<Actor>> actors;
    std::unique_ptr<studysteady::motion::TextureBridgeManager> renderer;
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
    std::unique_ptr<studysteady::motion::RuntimeGl> egl;
#endif
    std::string error;
    void checkThread() const {
        if(thread!=std::this_thread::get_id())throw std::runtime_error("motion runtime must be used on its owning thread");
    }
    void initialize() {
        for(const char *name:{"tjs2","core","plugin"})
            if(!spdlog::get(name))spdlog::stdout_color_mt(name);
        engine=new tTJS();motionSetTjsEngine(engine);
        // NCB's real AllRegist appends an index; build it once per process.
        // AllUnregist does not erase its module marker, so the host lifecycle
        // explicitly removes this module's marker after unregistering below.
        std::call_once(registrarIndexOnce,[]{ncbAutoRegister::AllRegist();});registered=true;
        if(!ncbAutoRegister::LoadModule(TJS_W("studysteady_motion_runtime.dll")))
            throw std::runtime_error("motion NCB class registration failed");
        motion::internal::initializeBezierPatchRuntime_guess();
        auto native=std::make_unique<ResourceManager>();
        auto *dispatch=ncbInstanceAdaptor<ResourceManager>::CreateAdaptor(native.get());
        if(!dispatch)throw std::runtime_error("motion ResourceManager adaptor creation failed");
        manager=native.release();managerOwner=tTJSVariant(dispatch,dispatch);dispatch->Release();
        if(ncbInstanceAdaptor<ResourceManager>::GetNativeInstance(managerOwner.AsObjectNoAddRef())!=manager)
            throw std::runtime_error("motion ResourceManager adaptor native identity mismatch");
        renderer=std::make_unique<studysteady::motion::TextureBridgeManager>(false);
        studysteady::motion::useRenderManager(renderer.get());
    }
    void close() {
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(egl)egl->makeCurrent();
#endif
        actors.clear();projects.clear();managerOwner.Clear();manager=nullptr;
        studysteady::motion::useRenderManager(nullptr);renderer.reset();
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        egl.reset();
#endif
        motionSetStorageReader(nullptr,nullptr,nullptr);
        motionSetPsbKeyResolver(nullptr,nullptr);
        motionSetPsbHeaderSeed(0);
        if(registered){ncbAutoRegister::AllUnregist();
            TVPRegisteredPlugins.erase(TJS_W("studysteady_motion_runtime.dll"));registered=false;}
        if(engine){engine->Shutdown();engine->Release();engine=nullptr;motionSetTjsEngine(nullptr);}
    }
    Project &project(uint64_t id) {
        const auto it=projects.find(id);
        if(it==projects.end())throw std::runtime_error("motion project id not found");
        return it->second;
    }
    Actor &actor(uint64_t id){const auto it=actors.find(id);if(it==actors.end())throw std::runtime_error("motion player id not found");return *it->second;}
    void current(){
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(egl)egl->makeCurrent();
#endif
        studysteady::motion::useRenderManager(renderer.get());
    }
    std::unique_ptr<Actor> createActor(uint64_t projectId){
        using namespace motion::detail;current();const auto &p=project(projectId);
        auto a=std::make_unique<Actor>(projectId,managerOwner);
        const auto metadata=motionPropGet(p.root,TJS_W("metadata"));const auto base=motionPropGet(metadata,TJS_W("base"));
        auto &player=a->engine->player();player.setProject(tTJSVariant(p.path));
        player.setChara(motionPropGetString(base,TJS_W("chara")));
        player.playMotion_guess(motion::PlayFlagForce,motionPropGetString(base,TJS_W("motion")));
        a->engine->applyMetadata_guess(metadata);a->engine->progress(0);return a;
    }
};
namespace {
template<class Function> bool invoke(StudyMotionRuntime *r,Function function) {
    if(!r)return false;
    try {
        r->checkThread();r->error.clear();
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        studysteady::motion::CurrentGlContextScope hostContext;
        r->current();
#endif
        function();
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        hostContext.restore();
#endif
        return true;
    }
    catch(const eTJSError &e){r->error=e.GetMessage().AsStdString();}
    catch(const std::exception &e){r->error=e.what();}
    catch(...){r->error="unknown motion runtime failure";}
    return false;
}
}
extern "C" StudyMotionRuntime *study_motion_create(char *error,size_t capacity) {
    std::lock_guard<std::mutex> lock(ownerMutex);
    if(ownerActive){copyError(error,capacity,"a motion runtime already owns process-wide TJS/NCB state");return nullptr;}
    auto r=std::make_unique<StudyMotionRuntime>();
    try {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        if(!SDL_IsMainThread())throw std::runtime_error("Create the SDL motion runtime on the main thread");
#endif
        r->initialize();ownerActive=true;copyError(error,capacity,"");return r.release();}
    catch(const eTJSError &e){copyError(error,capacity,e.GetMessage().AsStdString());}
    catch(const std::exception &e){copyError(error,capacity,e.what());}
    catch(...){copyError(error,capacity,"unknown motion initialization failure");}
    try {r->close();}catch(...){}
    return nullptr;
}
extern "C" int study_motion_destroy(StudyMotionRuntime *r,char *error,size_t capacity) {
    if(!r){copyError(error,capacity,"");return 1;}
    std::lock_guard<std::mutex> lock(ownerMutex);
    if(!invoke(r,[&]{r->close();})){copyError(error,capacity,r->error);return 0;}
    delete r;ownerActive=false;copyError(error,capacity,"");return 1;
}
extern "C" int study_motion_stats(StudyMotionRuntime *r,StudyMotionStats *stats) {
    return invoke(r,[&]{if(!stats)throw std::runtime_error("null stats output");
        stats->capabilities=capabilities;stats->project_count=static_cast<uint32_t>(r->projects.size());stats->player_count=static_cast<uint32_t>(r->actors.size());
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(r->egl)stats->capabilities|=STUDY_MOTION_GLES_RENDERER;
#endif
    });
}
extern "C" int study_motion_set_reader(StudyMotionRuntime *r,StudyMotionRead read,StudyMotionRelease release,void *user) {
    return invoke(r,[&]{
        if(!r->projects.empty())throw std::runtime_error("set motion reader before loading projects");
        motionSetStorageReader(read,release,user);
    });
}
extern "C" int study_motion_set_psb_key_resolver(StudyMotionRuntime *r,StudyMotionPsbKeyResolver resolver,void *user) {
    return invoke(r,[&]{
        if(!r->projects.empty())throw std::runtime_error("set motion PSB key resolver before loading projects");
        motionSetPsbKeyResolver(resolver,user);
    });
}
extern "C" uint64_t study_motion_load_project(StudyMotionRuntime *r,const char *path,uint32_t seed) {
    uint64_t result=0;
    invoke(r,[&]{
        if(!path || !*path)throw std::runtime_error("empty motion project path");
        const auto placed=TVPGetPlacedPath(ttstr(path));
        for(const auto &p:r->projects)if(p.second.path==placed){result=p.first;return;}
        motionSetPsbHeaderSeed(seed);
        auto root=r->manager->load(placed);
        if(root.Type()!=tvtObject)throw std::runtime_error("motion project load returned no dispatch");
        const auto id=r->nextProject;
        r->projects.emplace(id,StudyMotionRuntime::Project{placed,root});
        ++r->nextProject;result=id;
    });return result;
}
extern "C" int study_motion_unload_project(StudyMotionRuntime *r,uint64_t id) {
    return invoke(r,[&]{r->current();for(const auto &a:r->actors)if(a.second->project==id)throw std::runtime_error("destroy project players before unloading");const auto path=r->project(id).path;r->projects.erase(id);r->manager->unload(path);});
}
extern "C" int study_motion_project_base(StudyMotionRuntime *r,uint64_t id,char *chara,size_t nc,char *motion,size_t nm) {
    return invoke(r,[&]{using namespace motion::detail;
        const auto base=motionPropGet(motionPropGet(r->project(id).root,TJS_W("metadata")),TJS_W("base"));
        const auto c=motionPropGetString(base,TJS_W("chara"));
        const auto m=motionPropGetString(base,TJS_W("motion"));
        if(c.IsEmpty() || m.IsEmpty())throw std::runtime_error("motion metadata.base missing");
        copyIdentifier(chara,nc,c);copyIdentifier(motion,nm,m);
    });
}
extern "C" const char *study_motion_last_error(const StudyMotionRuntime *r) {
    return r?r->error.c_str():"null motion runtime";
}
extern "C" int study_motion_initialize_gles(StudyMotionRuntime *r,uintptr_t display,uintptr_t shared) {
    return invoke(r,[&]{
        if(!r->projects.empty()||!r->actors.empty())throw std::runtime_error("initialize GLES before loading projects");
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(r->egl)throw std::runtime_error("motion GLES already initialized");
        auto egl=std::make_unique<studysteady::motion::RuntimeGl>(display,shared);
        egl->makeCurrent();
        auto renderer=std::make_unique<studysteady::motion::GlesRenderManager>();
        r->renderer=std::move(renderer);r->egl=std::move(egl);r->current();
#else
        (void)display;(void)shared;throw std::runtime_error("motion GL renderer is not enabled in this build");
#endif
    });
}
extern "C" uint64_t study_motion_create_player(StudyMotionRuntime *r,uint64_t project) {
    uint64_t result=0;
    invoke(r,[&]{auto a=r->createActor(project);
        const auto id=r->nextActor;r->actors.emplace(id,std::move(a));++r->nextActor;result=id;
    });return result;
}
extern "C" uint64_t study_motion_clone_player(StudyMotionRuntime *r,uint64_t source){
    uint64_t result=0;invoke(r,[&]{auto &original=r->actor(source);auto copy=r->createActor(original.project);
        const auto state=original.engine->serializeState_guess();copy->engine->unserializeState_guess(state);
        copy->baseScale=original.baseScale;copy->userScale=original.userScale;
        // Publish the restored controller/variable state into the new Player
        // before its first draw, matching the state-restore API. No time passes.
        copy->engine->_dirty=true;copy->engine->progress(0);
        const auto id=r->nextActor;r->actors.emplace(id,std::move(copy));++r->nextActor;result=id;
    });return result;
}
extern "C" int study_motion_save_state(StudyMotionRuntime *r,uint64_t id,void **bytes,size_t *size){return invoke(r,[&]{
    if(!bytes||!size)throw std::runtime_error("null motion state output");*bytes=nullptr;*size=0;r->current();auto &a=r->actor(id);
    const auto buffer=studysteady::motion::encodeActorState(*a.engine,r->project(a.project).path,a.baseScale,a.userScale);
    auto *copy=std::malloc(buffer.size());if(!copy)throw std::bad_alloc();std::memcpy(copy,buffer.data(),buffer.size());*bytes=copy;*size=buffer.size();
});}
extern "C" void study_motion_free_buffer(void *bytes){std::free(bytes);}
extern "C" int study_motion_restore_state(StudyMotionRuntime *r,uint64_t id,const void *bytes,size_t size){return invoke(r,[&]{
    r->current();auto &original=r->actor(id);studysteady::motion::validateActorStateEnvelope(bytes,size,r->project(original.project).path);auto replacement=r->createActor(original.project);
    const auto state=studysteady::motion::decodeActorState(bytes,size,*replacement->engine,r->project(original.project).path);
    replacement->engine->unserializeState_guess(state.engine);replacement->engine->_dirty=true;replacement->engine->progress(0);
    replacement->baseScale=state.baseScale;replacement->userScale=state.userScale;replacement->serial=original.serial;
    replacement->target=original.target;original.target=nullptr;r->actors[id]=std::move(replacement);
});}
extern "C" int study_motion_destroy_player(StudyMotionRuntime *r,uint64_t id){return invoke(r,[&]{r->current();(void)r->actor(id);r->actors.erase(id);});}
extern "C" int study_motion_progress_player(StudyMotionRuntime *r,uint64_t id,double dt){return invoke(r,[&]{
    if(!std::isfinite(dt)||std::fabs(dt)>3600)throw std::runtime_error("motion delta out of range");r->current();r->actor(id).engine->progress(dt);
});}
extern "C" int study_motion_set_variable(StudyMotionRuntime *r,uint64_t id,const char *name,double value,double duration,double ease){return invoke(r,[&]{
    if(!name||!std::isfinite(value)||!std::isfinite(ease)||!std::isfinite(duration)||duration<0)throw std::runtime_error("invalid motion variable arguments");
    r->actor(id).engine->setVariable(ttstr(name),value,duration,ease);
});}
extern "C" int study_motion_get_variable(StudyMotionRuntime *r,uint64_t id,const char *name,double *value){return invoke(r,[&]{
    if(!name||!value)throw std::runtime_error("invalid motion variable output");*value=r->actor(id).engine->getVariable(ttstr(name));
});}
extern "C" int study_motion_play_timeline(StudyMotionRuntime *r,uint64_t id,const char *name,uint32_t flags){return invoke(r,[&]{
    if(!name)throw std::runtime_error("null motion timeline label");r->actor(id).engine->playTimeline_guess(ttstr(name),flags);
});}
extern "C" int study_motion_stop_timeline(StudyMotionRuntime *r,uint64_t id,const char *name){return invoke(r,[&]{
    if(!name)throw std::runtime_error("null motion timeline label");r->actor(id).engine->stopTimeline_guess(ttstr(name));
});}
extern "C" int study_motion_is_timeline_playing(StudyMotionRuntime *r,uint64_t id,const char *name,int *playing){return invoke(r,[&]{
    if(!name||!playing)throw std::runtime_error("invalid motion timeline query");*playing=r->actor(id).engine->isTimelinePlaying_guess(ttstr(name))?1:0;
});}
extern "C" int study_motion_timeline_info(StudyMotionRuntime *r,uint64_t id,int diff,uint32_t index,char *name,size_t capacity,double *duration,int *looping){return invoke(r,[&]{
    if((diff!=0&&diff!=1)||!duration||!looping)throw std::runtime_error("invalid timeline info output");
    const auto &engine=*r->actor(id).engine;const auto count=diff?engine.countDiffTimelines_guess():engine.countMainTimelines_guess();
    if(index>=static_cast<uint32_t>(count))throw std::runtime_error("motion timeline index out of range");
    const auto label=diff?engine.getDiffTimelineLabelAt_guess(index):engine.getMainTimelineLabelAt_guess(index);
    copyIdentifier(name,capacity,label);*duration=engine.getTimelineTotalFrameCount_guess(label);*looping=engine.getLoopTimeline_guess(label)?1:0;
});}
extern "C" int study_motion_timeline_position(StudyMotionRuntime *r,uint64_t id,const char *name,double *current){return invoke(r,[&]{
    if(!name||!current)throw std::runtime_error("invalid timeline position query");
    const auto &states=r->actor(id).engine->_timelineStates;const auto it=states.find(ttstr(name));
    if(it==states.end())throw std::runtime_error("motion timeline label not found");*current=it->second.currentTime;
});}
namespace {
float controllerEase(double duration,double ease){
    if(!std::isfinite(duration)||duration<0||duration>360000||!std::isfinite(ease)||std::fabs(ease)>1000000)throw std::runtime_error("invalid motion controller duration/ease");
    return static_cast<float>(ease);
}
}
extern "C" int study_motion_set_coord(StudyMotionRuntime *r,uint64_t id,double x,double y,double duration,double ease){return invoke(r,[&]{
    const auto power=controllerEase(duration,ease);
    if(!std::isfinite(x)||!std::isfinite(y)||std::fabs(x)>10000000||std::fabs(y)>10000000)throw std::runtime_error("invalid motion coordinate");
    auto &engine=*r->actor(id).engine;const float values[]={static_cast<float>(x),static_cast<float>(y)};engine._dirty=true;
    motion::EmoteVarController_setTarget_guess(engine._ctlPosition.get(),values,static_cast<float>(duration),power,engine._queuing);
});}
extern "C" int study_motion_set_scale(StudyMotionRuntime *r,uint64_t id,double scale,double duration,double ease){return invoke(r,[&]{
    const auto power=controllerEase(duration,ease);
    if(!std::isfinite(scale)||std::fabs(scale)>100000)throw std::runtime_error("invalid motion scale");
    auto &actor=r->actor(id);actor.userScale=static_cast<float>(scale);auto &engine=*actor.engine;const float value=actor.baseScale*actor.userScale;engine._dirty=true;
    motion::EmoteVarController_setTarget_guess(engine._ctlScale.get(),&value,static_cast<float>(duration),power,engine._queuing);
});}
extern "C" int study_motion_get_transform(StudyMotionRuntime *r,uint64_t id,StudyMotionTransform *out){return invoke(r,[&]{
    if(!out)throw std::runtime_error("null motion transform output");
    const auto &actor=r->actor(id);const auto &engine=*actor.engine;const auto &player=engine.player();
    *out={actor.baseScale,actor.userScale,engine._ctlScale->currentValue[0],
        engine._ctlPosition->currentValue[0],engine._ctlPosition->currentValue[1],
        player.getZoomX(),player.getZoomY(),player.getX(),player.getY(),player.getFrameTickCount()};
});}
extern "C" int study_motion_player_bounds(StudyMotionRuntime *r,uint64_t id,double *l,double *t,double *right,double *b){return invoke(r,[&]{using namespace motion::detail;
    if(!l||!t||!right||!b)throw std::runtime_error("null motion bounds output");auto &p=r->actor(id).engine->player();p.calcBoundsForDifferentialTest_guess();const auto box=p.getBounds();
    *l=motionPropGetDouble(box,TJS_W("left"));*t=motionPropGetDouble(box,TJS_W("top"));*right=motionPropGetDouble(box,TJS_W("right"));*b=motionPropGetDouble(box,TJS_W("bottom"));
});}
extern "C" int study_motion_render_player(StudyMotionRuntime *r,uint64_t id,uint32_t w,uint32_t h,const double *matrix,StudyMotionFrame *frame){return invoke(r,[&]{
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
    if(!r->egl)throw std::runtime_error("motion GLES is not initialized");
    if(!frame||w<1||h<1||w>8192||h>8192||uint64_t(w)*h*4>128*1024*1024)throw std::runtime_error("motion render size/output invalid");
    const double identity[]={1,0,0,1,0,0};if(!matrix)matrix=identity;
    for(int i=0;i<6;++i)if(!std::isfinite(matrix[i]))throw std::runtime_error("nonfinite motion render transform");
    r->current();auto &a=r->actor(id);
    // A restored/static Sprite can be drawn before its first timer callback.
    // Setters update real controller values and mark Engine dirty; publish
    // those values without advancing its timeline before preparing geometry.
    if(a.engine->_dirty)a.engine->progress(0);
    auto &p=a.engine->player();
    auto &renderer=dynamic_cast<studysteady::motion::GlesRenderManager &>(*r->renderer);
    if(!a.target||a.target->GetWidth()!=w||a.target->GetHeight()!=h){
        auto *replacement=renderer.CreateTexture2D(nullptr,0,w,h,TVPTextureFormat::RGBA,0);
        if(a.target)a.target->Release();a.target=replacement;
    }
    p.setDrawAffineTranslateMatrix(matrix[0],matrix[1],matrix[2],matrix[3],matrix[4],matrix[5]);
    motion::detail::PreparedRenderItemList main,aux;
    if(!p.prepareRenderItemsForDifferentialTest_guess(main,aux))throw std::runtime_error("motion player has no renderable content");
    renderer.clearTarget(a.target);auto *target=a.target;
    studysteady::motion::renderSceneItems(false,target,
        [target](bool,const tTVPRect &){return motion::D3DTargetTexturePair_guess(target,target);},
        {0,0,static_cast<int>(w),static_cast<int>(h)},[](motion::detail::PreparedRenderItem &item){
            if(!item.sourceState->texture)throw std::runtime_error("motion source texture missing");return item.sourceState->texture;
        },main,.5f,.5f);
    glFinish();if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("motion shared texture synchronization failed");
    *frame={studysteady::motion::textureGlesName(target),w,h,++a.serial,1,1};
#else
    (void)id;(void)w;(void)h;(void)matrix;(void)frame;throw std::runtime_error("motion GL renderer is not enabled in this build");
#endif
});}
extern "C" int study_motion_read_pixels(StudyMotionRuntime *r,uint64_t id,void *rgba,size_t capacity){return invoke(r,[&]{
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
    if(!r->egl)throw std::runtime_error("motion GLES is not initialized");r->current();auto &a=r->actor(id);
    if(!a.target||!rgba||capacity<size_t(a.target->GetWidth())*a.target->GetHeight()*4)throw std::runtime_error("motion pixel output buffer invalid");
    const auto pixels=dynamic_cast<studysteady::motion::GlesRenderManager &>(*r->renderer).readTarget(a.target);std::memcpy(rgba,pixels.data(),pixels.size());
#else
    (void)id;(void)rgba;(void)capacity;throw std::runtime_error("motion GL renderer is not enabled in this build");
#endif
});}
