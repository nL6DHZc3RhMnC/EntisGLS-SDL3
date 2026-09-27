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
#include "../legacy_environment.h"
#include "../legacy_thread.h"
#include "../legacy_script_file.h"
#include "../legacy_resource.h"
#include "../legacy_sprite.h"
#include "../legacy_resource_manager.h"
#include "../legacy_input.h"
#include "../legacy_window.h"
#include "../legacy_setup.h"
#include "../legacy_message.h"
#include "../legacy_emote.h"
#include "../legacy_super_sprite.h"
#include "../legacy_particle.h"
#include "../legacy_tone_filter.h"
#include "../legacy_compiler.h"
#include "../legacy_audio_player.h"
#include "../legacy_movie.h"
#include "../legacy_native_binding.h"
#include <glscs_execution_image_linker.h>
#include <glscs_execution_image_compiler.h>
#include <glscs_assembler.h>
#include <glscs_compiler.h>
