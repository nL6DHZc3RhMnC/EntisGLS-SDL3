#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// No TJS, Cotopha, engine SDK, JNI, or GLES types cross this boundary.
typedef struct StudyMotionRuntime StudyMotionRuntime;
enum StudyMotionCapability {
    STUDY_MOTION_REAL_TJS = 1u << 0,
    STUDY_MOTION_REAL_NCB = 1u << 1,
    STUDY_MOTION_PSB_V4 = 1u << 2,
    STUDY_MOTION_PLAYER_CPU = 1u << 3,
    STUDY_MOTION_GLES_RENDERER = 1u << 4
};
typedef struct StudyMotionStats {
    uint32_t capabilities;
    uint32_t project_count;
    uint32_t player_count;
} StudyMotionStats;
typedef struct StudyMotionFrame {
    uint32_t texture; // borrowed GL_TEXTURE_2D; caller must never delete it
    uint32_t width,height;
    uint64_t serial;
    uint32_t texture_origin_bottom_left; // 1 for the rendered FBO texture
    uint32_t premultiplied_alpha; // 1: RGB already weighted by alpha
} StudyMotionFrame;
// Reader returns 1=found/success, 0=not found, -1=error (write error buffer).
// When bytes==NULL, perform an existence/size query without allocating.
// Otherwise return the still-encrypted PSB bytes and length. release is called
// exactly once for every non-NULL returned buffer, including failed reads.
// Callbacks must not throw; their user object must live until reader replacement
// or owner destruction. They execute synchronously on the owner thread.
typedef int (*StudyMotionRead)(void *user,const char *path,void **bytes,size_t *size,
    char *error,size_t error_capacity);
typedef void (*StudyMotionRelease)(void *user,void *bytes,size_t size);
// Resolve an encrypted PSB header key from the complete, still-encrypted file.
// psb_bytes is read-only borrowed memory, valid only for this synchronous call;
// do not retain it or modify it. Return 1 and write key on success; otherwise
// return 0/-1 and write a diagnostic (never the key or source file contents).
// Unencrypted PSBs skip this callback and still undergo header validation.
// Like the reader, this callback must not throw and runs on the owner thread.
typedef int (*StudyMotionPsbKeyResolver)(void *user,const char *path,
    const void *psb_bytes,size_t psb_size,uint32_t *key,char *error,size_t error_capacity);

// One owner per process; every call and final release must run on its creator
// thread. This initializes genuine TJS, NCB classes and ResourceManager only.
// SDL builds require that owner to be the SDL main thread.
// Query capabilities before promising a renderer to a game script.
StudyMotionRuntime *study_motion_create(char *error, size_t error_capacity);
// Returns zero without destroying the owner when invoked on the wrong thread.
int study_motion_destroy(StudyMotionRuntime *runtime, char *error, size_t error_capacity);
int study_motion_stats(StudyMotionRuntime *runtime, StudyMotionStats *stats);
// Set before loading projects. Passing both callbacks NULL restores filesystem
// storage. Decode/adoption remains inside the single real PSB reader.
int study_motion_set_reader(StudyMotionRuntime *runtime,StudyMotionRead read,
    StudyMotionRelease release,void *user);
// Set before loading projects. The user object must remain alive until callback
// replacement or owner destruction. NULL restores explicit load_project seeds.
// With a resolver installed, encrypted loads use only its result and never fall
// back to the explicit seed after resolution or header validation fails.
int study_motion_set_psb_key_resolver(StudyMotionRuntime *runtime,
    StudyMotionPsbKeyResolver resolver,void *user);
// Real ResourceManager filesystem load; input retains its encrypted PSB header.
// A nonzero id owns a genuine PSB dispatch until unload/destroy. Id zero fails.
// header_seed is used only when no key resolver is installed.
uint64_t study_motion_load_project(StudyMotionRuntime *runtime, const char *path, uint32_t header_seed);
int study_motion_unload_project(StudyMotionRuntime *runtime, uint64_t project_id);
// Copies the project's actual metadata.base chara/motion in UTF-8. Too-small
// buffers fail explicitly instead of returning truncated identifiers.
int study_motion_project_base(StudyMotionRuntime *runtime, uint64_t project_id,
    char *chara, size_t chara_capacity, char *motion, size_t motion_capacity);
// Last error remains owned by runtime and valid until its next API call.
const char *study_motion_last_error(const StudyMotionRuntime *runtime);
// Original Android backend: creates a pbuffer/context on the owning worker and shares textures
// with the supplied EGLDisplay/EGLContext handles (passed as uintptr_t).
// Both zero selects a standalone EGL context for probes. SDL builds instead
// accept SDL_Window*/SDL_GLContext, require both, and create a shared context
// on the main thread. Every SDL API call restores the caller's current context.
// Call before loading any project; the runtime owns only its new context.
// The supplied parent window/context must remain alive until runtime destruction.
int study_motion_initialize_gles(StudyMotionRuntime *runtime,uintptr_t egl_display,uintptr_t shared_context);
// Actual EmoteEngine: force-play metadata.base, apply full metadata/selectors,
// then progress(0). Requires a loaded project; zero id indicates failure.
uint64_t study_motion_create_player(StudyMotionRuntime *runtime,uint64_t project_id);
// Same-project Engine clone using its genuine serialize/unserialize methods.
// This preserves the state defined by the original engine serializer, not all
// arbitrary variables or transient physics. Advance before rendering the clone.
uint64_t study_motion_clone_player(StudyMotionRuntime *runtime,uint64_t player_id);
// Genuine TJS structured-binary snapshot with a version/project envelope.
// On success save allocates *bytes; release it with study_motion_free_buffer.
// Restore validates size/tree/schema/controller labels, builds a replacement
// Engine, and commits only on success. Actor id and existing texture stay valid.
// It preserves the original serializer's state surface (see clone notes above).
int study_motion_save_state(StudyMotionRuntime *runtime,uint64_t player_id,void **bytes,size_t *size);
void study_motion_free_buffer(void *bytes);
int study_motion_restore_state(StudyMotionRuntime *runtime,uint64_t player_id,const void *bytes,size_t size);
int study_motion_destroy_player(StudyMotionRuntime *runtime,uint64_t player_id);
// Delta and duration use the original 60-Hz frame domain, not milliseconds.
int study_motion_progress_player(StudyMotionRuntime *runtime,uint64_t player_id,double delta_frames);
int study_motion_set_variable(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,double value,double duration_frames,double ease);
int study_motion_get_variable(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,double *value);
int study_motion_play_timeline(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,uint32_t flags);
int study_motion_stop_timeline(StudyMotionRuntime *runtime,uint64_t player_id,const char *name);
int study_motion_is_timeline_playing(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,int *playing);
// Duration uses 60-Hz frames; ease is passed through unchanged. Any nonzero
// auto_stop enables the engine's automatic timeline stop after blending.
int study_motion_set_timeline_blend(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,double value,double duration_frames,double ease,int auto_stop);
// Uses the engine's native controller reset/skip behavior.
int study_motion_skip(StudyMotionRuntime *runtime,uint64_t player_id);
// Legacy Cotopha's unified physics weight controls the simple springs only;
// hair and parts scales are unchanged. Values retain native float precision.
int study_motion_set_physics_weight(StudyMotionRuntime *runtime,uint64_t player_id,double weight);
// Enumerates actual metadata timeline labels (diff=0 main, diff=1 differential).
// A missing index fails explicitly. Duration is the original frame domain.
int study_motion_timeline_info(StudyMotionRuntime *runtime,uint64_t player_id,int diff,uint32_t index,char *name,size_t capacity,double *duration,int *looping);
int study_motion_timeline_position(StudyMotionRuntime *runtime,uint64_t player_id,const char *name,double *current_frame);
// Actual D3DEmotePlayer root controllers. ease is passed through unchanged,
// matching the game's native interface (it supplies zero); no TJS-script
// ease-to-power conversion. The parent's stage transform owns viewport scale.
int study_motion_set_coord(StudyMotionRuntime *runtime,uint64_t player_id,double x,double y,double duration_frames,double ease);
int study_motion_set_scale(StudyMotionRuntime *runtime,uint64_t player_id,double scale,double duration_frames,double ease);
// Read-only current controller/published Player transform, without stepping or
// consuming an animation keyframe. Used to distinguish host and engine state.
typedef struct StudyMotionTransform {
    double base_scale,user_scale;
    double controller_scale,controller_x,controller_y;
    double player_scale_x,player_scale_y,player_x,player_y;
    double frame;
} StudyMotionTransform;
int study_motion_get_transform(StudyMotionRuntime *runtime,uint64_t player_id,StudyMotionTransform *transform);
int study_motion_player_bounds(StudyMotionRuntime *runtime,uint64_t player_id,double *left,double *top,double *right,double *bottom);
// Affine order: m11,m21,m12,m22,tx,ty; NULL means identity. Rendering preserves
// the Player's actual geometry/priority/stencils and does not fit automatically.
// The published texture is synchronized before return and stays owned by this
// player until its next size change or destruction. Requires GLES capability.
int study_motion_render_player(StudyMotionRuntime *runtime,uint64_t player_id,uint32_t width,uint32_t height,const double *affine6,StudyMotionFrame *frame);
// Optional premultiplied RGBA8 fallback; rows start at top, pitch=width*4.
int study_motion_read_pixels(StudyMotionRuntime *runtime,uint64_t player_id,void *rgba,size_t capacity);
#ifdef __cplusplus
}
#endif
