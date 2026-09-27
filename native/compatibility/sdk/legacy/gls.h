#pragma once
#include <windows.h>
#include <eritypes.h>
#include <esl.h>
#include <sakura/sakura.h>
#include <sakura/ssys_module.h>

// Core plugin declarations do not pull in the legacy Win32 renderer. Platform
// graphics interfaces remain opaque until their concrete Android port exists.
#define __EGL2D_H__ 1
class EWaveMixingServer;
class EGLDrawImage;
class ECSEnvironment;
class EMCFile;
#include <glsscript.h>
#include <legacy_emc.h>
#include "runtime/cotopha_port/legacy_environment.h"
#include "runtime/cotopha_port/legacy_thread.h"
#include "runtime/cotopha_port/legacy_script_file.h"
#include "runtime/cotopha_port/legacy_resource.h"
#include "runtime/cotopha_port/legacy_sprite.h"
#include "runtime/cotopha_port/legacy_resource_manager.h"
#include "runtime/cotopha_port/legacy_input.h"
#include "runtime/cotopha_port/legacy_window.h"
#include "runtime/cotopha_port/legacy_setup.h"
#include "runtime/cotopha_port/legacy_message.h"
#include "runtime/cotopha_port/legacy_emote.h"
#include "runtime/cotopha_port/legacy_super_sprite.h"
#include "runtime/cotopha_port/legacy_particle.h"
#include "runtime/cotopha_port/legacy_tone_filter.h"
#include "runtime/cotopha_port/legacy_compiler.h"
#include "runtime/cotopha_port/legacy_audio_player.h"
#include "runtime/cotopha_port/legacy_movie.h"
#include "runtime/cotopha_port/legacy_native_binding.h"
#include <glscs_execution_image_linker.h>
#include <glscs_execution_image_compiler.h>
#include <glscs_assembler.h>
#include <glscs_compiler.h>
