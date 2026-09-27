#!/usr/bin/env python3
"""Separate imported Player CPU compilation from unrelated renderer includes.

All function bodies are retained. The Layer-dispatch resolver is declared as a
renderer integration requirement, not given an empty implementation.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
from motion_prepare_math import extract
SOURCE = ROOT / 'vendor/kirikiroid2/motionplayer'
OUT = ROOT / 'build/generated/motion-player'


def write_changed(path, text):
    if not path.exists() or path.read_text() != text:
        path.write_text(text)

def main():
    for src in SOURCE.rglob('*'):
        if not src.is_file() or src.suffix not in ('.h','.cpp'): continue
        text = src.read_text()
        text = text.replace('#include "../../core/visual/ComplexRect.h"','#include "ComplexRect.h"')
        text = text.replace('#include "common/LogoTrace.h"','#include "LogoTrace.h"')
        for header in ['ScriptMgnIntf.h','DebugIntf.h']:
            text = text.replace('#include "' + header + '"','#include "tjs_host.h"')
        if src.name in ('PlayerCore.cpp', 'PlayerFrameProgress.cpp'):
            text = text.replace('#include "EmotePlayer.h"','#include "EmoteEngine.h"')
        if src.name == 'EmoteEngine.cpp':
            text = text.replace('#include "EmotePlayer.h"', '#include "ResourceManager.h"')
            text = text.replace('#include "MsgIntf.h"', '#include "tjs_host.h"')
            # The native constructors deliberately leave idle interpolation
            # scalars unwritten. Active step paths initialize them before use,
            # but the source serializers read them unconditionally. Canonical
            # idle snapshot values must branch BEFORE evaluating those members;
            # active interpolation and all live controller code stay unchanged.
            idle_serializers = [
                ('tTJSVariant serializeVarControllerState_guess(', 'state',
                 {'phase':'0.0f','invDuration':'0.0f','powCount':'1.0f'}),
                ('tTJSVariant serializeAngleControllerState_guess(', 'state',
                 {'phase':'0.0f','invDuration':'0.0f','powCount':'1.0f',
                  'startRad':'controller->currentRad','targetRad':'controller->currentRad'}),
                ('void serializeEyeControllerState(', 'trackState',
                 {'trackSpan':'0.0f','trackAccum':'0.0f','trackInvDur':'0.0f','trackPow':'1.0f'}),
                ('void serializeEyebrowControllerState(', 'trackState',
                 {'trackSpan':'0.0f','trackAccum':'0.0f','trackInvDur':'0.0f','trackPow':'1.0f'}),
                ('void serializeMouthControllerState(', 'state',
                 {'accum':'0.0f','invDur':'0.0f','powField':'1.0f','startVal':'controller->currentValue'}),
            ]
            for signature, gate, fields in idle_serializers:
                body = extract(text, signature)
                adapted = body
                for field, default in fields.items():
                    member = 'controller->' + field
                    assert adapted.count(member) == 1, (signature, member)
                    adapted = adapted.replace(member, '(controller->' + gate + ' == 0 ? ' + default + ' : ' + member + ')')
                text = text.replace(body, '// Android snapshot fix: do not read indeterminate idle curve fields.\n        ' + adapted)
        if src.name == 'PlayerFrameProgress.cpp':
            text = '#include "player_mesh_bridge.h"\n' + text
            marker = 'void Player::frameProgress(double dt) {'
            assert text.count(marker) == 1
            text = text.replace(marker, marker + '\n        studysteady::motion::updatePlayerCombinatorsFromBindings(*this, *_rootPlayer);')
        if src.name == 'MotionNode.h':
            text = '#include <memory>\n#include "mesh_combinator.h"\n' + text
            marker = 'tTJSVariant frameListVariant;'
            assert text.count(marker) == 1
            text = text.replace(marker, marker + '\n        std::shared_ptr<studysteady::motion::MeshCombinator> meshCombinator;')
            assert text.count('std::array<int, 4> textureRect;') == 1
            text = text.replace('std::array<int, 4> textureRect;', 'std::array<double, 4> textureRect;')
        if src.name == 'MotionRenderBackend.h':
            text = '#include "atlas_projection.h"\n' + text
            assert text.count('const tTVPRect &sourceRect') == 1
            text = text.replace('const tTVPRect &sourceRect',
                                'const studysteady::motion::AtlasSampleRect &sourceRect')
        if src.name == 'NodeTree.cpp':
            text = '#include "player_mesh_bridge.h"\n' + text
            body = extract(text, 'void initializeNodeFromLayer_guess(')
            adapted = body[:-1] + '\n            studysteady::motion::bindNodeMeshCombinator(node, rawLayer);\n        }'
            text = text.replace(body, adapted)
        if src.name == 'PlayerUpdateLayerEval.cpp':
            text = '#include "player_mesh_bridge.h"\n' + text
            marker = 'evaluateTimeline_guess(detail::MotionNode &node,'
            assert text.count(marker) == 1
            text = text.replace(marker, 'evaluateTimelineLegacy_guess(detail::MotionNode &node,')
            text += '''
namespace motion::internal {
bool evaluateTimeline_guess(detail::MotionNode &node, double time, bool dirty) {
    const bool changed = evaluateTimelineLegacy_guess(node, time, dirty);
    studysteady::motion::publishNodeMeshCombinator(node);
    return changed;
}
}
'''
        if src.name == 'PlayerInternal.h':
            for header in ['WindowIntf.h','LayerIntf.h','LayerBitmapIntf.h','GraphicsLoaderIntf.h','tvpgl.h','D3DAdaptor.h','StorageIntf.h','EventIntf.h']:
                text = text.replace('#include "' + header + '"','')
            body = extract(text,'inline iTJSDispatch2 *tryResolveLayerDispatch(')
            text = text.replace(body,'iTJSDispatch2 *tryResolveLayerDispatch(const tTJSVariant &value);')
        target = OUT / src.relative_to(SOURCE)
        target.parent.mkdir(parents=True,exist_ok=True)
        write_changed(target, '// Generated include-boundary adaptation; original methods retained.\n' + text)
    separate = (SOURCE / 'SeparateLayerAdaptor.cpp').read_text()
    pieces = [extract(separate, name) for name in [
        'void invalidateObjectVariant_guess(',
        'void invalidatePayloadLayerVariant_guess(']]
    ownership = '#include "SeparateLayerAdaptor.h"\nnamespace {\n' + '\n'.join(pieces) + '\n}\nnamespace motion {\n'
    ownership += '\n'.join(extract(separate, name) for name in [
        'SeparateLayerOrderedMap_guess::~SeparateLayerOrderedMap_guess()',
        'void SeparateLayerOrderedMap_guess::clear(',
        'SeparateLayerAdaptor::~SeparateLayerAdaptor()',
        'void SeparateLayerAdaptor::clearPrivateRenderState()',
        'void SeparateLayerAdaptor::clear()'])
    ownership += '\n}\n'
    write_changed(OUT / 'SeparateLayerOwnership.cpp', ownership)
    resource = (SOURCE / 'ResourceManager.cpp').read_text()
    start = resource.index('namespace motion::detail {')
    end = resource.index('namespace {',start)
    core = '#include "ResourceManager.h"\n#include "MotionDispatch.h"\n#include "RuntimeSupport.h"\n#include "RenderManager.h"\n#include "psbfile/PSBDispatch.h"\n#include "ncbind.hpp"\n#include "psb_storage.h"\n'
    core += resource[start:end]
    core += 'namespace {\n' + extract(resource,'void initializeRandomGenerator(') + '\nPSB::PSBFile::OwnerFilter emotePSBDecryptFilter;\n}\n'
    core += '\n'.join(extract(resource, name) for name in [
        'motion::ResourceManager::ResourceManager()',
        'motion::ResourceManager::~ResourceManager()',
        'tTJSVariant motion::ResourceManager::load(',
        'void motion::ResourceManager::unload(',
        'void motion::ResourceManager::unloadAll()',
        'tjs_int motion::ResourceManager::requireLayerId()',
        'tjs_int motion::ResourceManager::releaseLayerId(',
        'bool motion::ResourceManager::isExistMotion(',
        'tTJSVariant motion::ResourceManager::findMotion(',
        'double motion::ResourceManager::random()'])
    core += '\nnamespace motion { SourceCache::~SourceCache() = default; }\n'
    write_changed(OUT / 'ResourceManagerCpu.cpp', core)
    query = (SOURCE / 'PlayerLayerQuery.cpp').read_text()
    query_core = '#include \"PlayerInternal.h\"\nnamespace motion {\n'
    query_core += '\n'.join(extract(query, name) for name in [
        'void Player::setZFactor(', 'void Player::visitChildPlayerDispatches_guess(',
        'detail::MotionNode *Player::findNodeByRawLabel_guess(',
        'void Player::setOpacity(', 'void Player::setVisible(', 'void Player::setFlip(',
        'void Player::setZoom(', 'void Player::setSlant(', 'void Player::setFlipX('])
    query_core += '\n}\n'
    write_changed(OUT / 'PlayerQueryCpu.cpp', query_core)
    query_scene = (OUT / 'PlayerLayerQuery.cpp').read_text()
    for name in ['void Player::setZFactor(', 'void Player::visitChildPlayerDispatches_guess(',
        'detail::MotionNode *Player::findNodeByRawLabel_guess(', 'void Player::setOpacity(',
        'void Player::setVisible(', 'void Player::setFlip(', 'void Player::setZoom(',
        'void Player::setSlant(', 'void Player::setFlipX(']:
        query_scene = query_scene.replace(extract(query_scene, name), '')
    write_changed(OUT / 'PlayerLayerQueryScene.cpp', query_scene)
    emote_header = (SOURCE / 'EmotePlayer.h').read_text()
    accessors = '#include "EmoteEngine.h"\n#include "Player.h"\nnamespace motion {\n'
    accessors += '\n'.join(extract(emote_header, name).removeprefix('inline ') for name in [
        'inline Player& EmoteEngine::player()', 'inline const Player& EmoteEngine::player() const'])
    write_changed(OUT / 'EmotePlayerAccessors.cpp', accessors + '\n}\n')
    registration = (SOURCE / 'main.cpp').read_text()
    geometry = '#include "SourceCache.h"\n'
    for cls in ['Point','Circle','Rect','Quad','LayerGetter']:
        geometry += 'using motion::' + cls + ';\n'
        geometry += extract(registration, 'NCB_REGISTER_SUBCLASS_DELAY(' + cls + ')').replace('NCB_REGISTER_SUBCLASS_DELAY(', 'NCB_REGISTER_CLASS(') + '\n'
    write_changed(OUT / 'MotionGeometryRegistration.inc', geometry)
    render_items = (SOURCE / 'PlayerRenderItems.cpp').read_text()
    colors = '#include "PlayerInternal.h"\nnamespace {\n'
    colors += extract(render_items, 'inline std::uint32_t packedColorInterpolationWeightS32_guess(')
    colors += '\n}\nnamespace motion::internal {\n'
    colors += extract(render_items, 'std::uint32_t interpolatePackedColor_guess(')
    colors += '\n}\n'
    write_changed(OUT / 'PlayerTimelineColor.cpp', colors)
    timeline = (SOURCE / 'PlayerTimeline.cpp').read_text()
    timeline_core = '#include "PlayerInternal.h"\nusing namespace motion::internal;\nnamespace motion {\nnamespace {\n'
    timeline_core += extract(timeline, 'struct TimelineDispatchReleaseGuard_guess') + ';\n}\n'
    timeline_core += '\n'.join(extract(timeline, name) for name in [
        'void Player::skipToSync()', 'void Player::playMotion_guess(',
        'void Player::playMotionImpl_guess(', 'tjs_error Player::playCompat(', 'void Player::stop()'])
    timeline_core += '\n}\n'
    write_changed(OUT / 'PlayerTimelineCore.cpp', timeline_core)
    source = (SOURCE / 'PlayerResource.cpp').read_text()
    win = '#include "PlayerInternal.h"\n#include "RenderManager.h"\n#include "tjsUtils.h"\n#include "texture_bridge.h"\nusing namespace motion::internal;\nnamespace motion {\nnamespace {\n'
    win += '\n'.join(extract(source, name) for name in [
        'constexpr tjs_int signedW32(', 'struct DispatchRelease_guess',
    ]).replace('};','} ;') + ';\n'
    win += 'using RetainedDispatch_guess = std::unique_ptr<iTJSDispatch2, DispatchRelease_guess>;\ntjs_uint32 findSourceMemberHint_guess = 0;\n'
    win += '\n'.join(extract(source, name) for name in [
        'RetainedDispatch_guess retainVariantObject_guess(',
        'bool findWinSourceGroup_guess(', 'iTVPTexture2D *loadWinAtlasTexture_guess('])
    win += '\n}\n'
    find_source = extract(source, 'void Player::findSourceForNode_guess(')
    # The old implementation AddRefs this raw AsObject receiver but never
    # releases it. The Android owner must be destroyable after source loading.
    # Pair that acquisition without changing any source selection/texture math.
    acquisition = '_findSourceResourceManager.AsObject();'
    find_source = find_source.replace(acquisition, acquisition + '\n        RetainedDispatch_guess resourceManagerOwner(resourceManagerDispatch);')
    # Win PSB icons can be packed at a lower pixel resolution while retaining
    # their original logical dimensions and origins for geometry/animation.
    # Native drivers read an optional float resolution (default 1.0); the
    # imported Kirikiri projection previously discarded that field entirely.
    old_rect = '''                        dst.textureRect = {
                            left, top,
                            static_cast<int>(
                                dst.width + static_cast<double>(left)),
                            static_cast<int>(
                                dst.height + static_cast<double>(top))
                        };'''
    assert find_source.count(old_rect) == 1
    find_source = find_source.replace(old_rect, '''                        PSB::PSBRawNode resolutionNode;
                        const double resolution = iconNode.GetDictionaryValue("resolution", resolutionNode)
                            ? resolutionNode.GetDouble() : 1.0;
                        dst.textureRect = studysteady::motion::winAtlasSampleRect(
                            left, top, dst.width, dst.height, resolution,
                            dst.texture->GetWidth(), dst.texture->GetHeight());''')
    for component in ('width', 'height'):
        # The unrelated Layer route retains its existing integer conversion,
        # then explicitly widens into the shared floating-point UV container.
        find_source = find_source.replace('static_cast<int>(dst.' + component + ')',
            'static_cast<double>(static_cast<int>(dst.' + component + '))')
    win = '#include "atlas_projection.h"\n' + win
    win += find_source + '\n'
    win += '\n'.join(extract(source,name) for name in [
        'bool Player::isExistMotion(',
        'void Player::releaseLayerId(', 'tjs_int Player::dispatchRequireLayerId(',
        'void Player::dispatchReleaseLayerId('])
    win += '\n' + extract((SOURCE / 'PlayerRender.cpp').read_text(), 'tjs_error Player::dispatchFindSource_guess(')
    win += '''
bool Player::loadKrkrAtlasSource_guess(detail::MotionNode::SourceState &, ResourceManager *, const ttstr &) {
    throw std::runtime_error("Android motion adapter currently supports spec=win atlas sources only");
}
}
'''
    write_changed(OUT / 'PlayerResourceWin.cpp', win)
    blank = extract(resource, 'tTJSVariant motion::ResourceManager::findSource(')
    blank = blank[:blank.index('    // Both direct vector elements')] + '    throw std::runtime_error("Generic src/ObjSource requires an unimplemented Layer texture adapter");\n}\n'
    write_changed(OUT / 'ResourceManagerBlank.cpp', '#include "ResourceManager.h"\n#include "RuntimeSupport.h"\n#include "MotionDispatch.h"\n#include "ncbind.hpp"\n#include <stdexcept>\n' + blank)
    items = (OUT / 'PlayerRenderItems.cpp').read_text()
    items = items.replace('#include "PlayerRenderInternal.h"','')
    items = items.replace(extract(items,'std::uint32_t interpolatePackedColor_guess('),'')
    write_changed(OUT / 'PlayerRenderItemsScene.cpp', items)
    backend = (SOURCE / 'MotionRenderBackend.cpp').read_text()
    backend = backend.replace('#include "LayerBitmapIntf.h"','')
    backend = backend.replace('#include "ogl/ogl_common.h"','#include "platform/gl.h"')
    backend = backend.replace('#include <GLES2/gl2.h>', '#include "platform/gl.h"')
    backend = backend.replace(extract(backend, 'tTVPBitmap *makeRepeatedSoftwareBitmap_guess('), '')
    repeat = extract(backend, 'if(sourceLeft < 0 || sourceTop < 0 ||')
    backend = backend.replace(repeat, '''if(sourceLeft < 0 || sourceTop < 0 ||
           sourceRect.right > static_cast<int>(sourceTexture->GetWidth()) ||
           sourceRect.bottom > static_cast<int>(sourceTexture->GetHeight())) {
            sourceTexture->Release();
            throw std::runtime_error("Repeated/out-of-atlas source rectangle unsupported by GLES bridge");
        }''')
    assert backend.count('const tTVPRect &sourceRect') == 1
    backend = backend.replace('const tTVPRect &sourceRect',
                              'const studysteady::motion::AtlasSampleRect &sourceRect')
    for declaration in ['const int sourceWidth', 'const int sourceHeight', 'int sourceLeft', 'int sourceTop']:
        assert backend.count(declaration) == 1, declaration
        backend = backend.replace(declaration, declaration.replace('int ', 'double '))
    backend = '#include "tjs.h"\n#include <stdexcept>\n' + backend
    # Managers now belong to opaque device owners, not a process-lifetime
    # singleton. Resolve method instances afresh; retain all math/batch logic.
    backend = backend.replace('static iTVPRenderManager *manager =', 'iTVPRenderManager *manager =')
    backend = backend.replace('static iTVPRenderMethod *method =', 'iTVPRenderMethod *method =')
    backend += '\nnamespace motion::render_backend_guess { std::map<int, CubicBezierBasisTable_guess> cubicBezierBasisCache_guess; }\n'
    write_changed(OUT / 'MotionRenderBackendNative.cpp', backend)
    targets = (SOURCE / 'PlayerRenderTargets.cpp').read_text()
    render = '#include "gles_scene_bridge.h"\n#include "PlayerInternal.h"\n#include "MotionRenderBackend.h"\n#include "RenderManager.h"\n#include <stdexcept>\nusing namespace motion;\nusing namespace motion::internal;\nnamespace studysteady::motion {\nnamespace {\nusing PreparedRenderItem = ::motion::detail::PreparedRenderItem;\n'
    render += extract((SOURCE / 'PlayerRenderInternal.cpp').read_text(), 'std::array<tjs_int, 2> renderBezierPatchCellDivisions_guess(')
    render += '\n'.join(extract(targets, name) for name in [
        'std::array<tTVPPointD, 6> makeTextureQuad(',
        'std::array<tTVPPointD, 6> makeAffineTargetQuad(',
        'std::vector<tTVPPointD> buildOffsetMeshPoints(',
        'unsigned int d3dPackedColorWithOpacity(',
        'bool markD3DStencilMaskChain_guess(',
        'int assignD3DStencilRefs_guess(', 'bool computeD3DClip_guess(',
        'int prepareD3DRenderItems_guess(', 'void appendD3DAffine_guess(',
        'void appendD3DMesh_guess(', 'bool shouldSkipD3DRenderItem_guess('])
    render = render.replace('(void)TVPShowSimpleMessageBox(message, caption);', 'throw std::runtime_error("Motion stencil reference overflow");')
    render += '\n}\n'
    routine = extract(targets, 'void Player::renderPreparedItemsToD3DTexture_guess(\n        iTVPTexture2D *targetTexture,')
    routine = routine.replace('void Player::renderPreparedItemsToD3DTexture_guess(', 'void renderSceneItems(bool priorDraw,')
    routine = routine.replace('_priorDraw','priorDraw').replace('motion::render_backend_guess', '::motion::render_backend_guess')
    render += routine + '\n}\n'
    # In this adapter's enclosing namespace, explicitly qualify original
    # motion::detail/render_backend names rather than changing their types.
    render = render.replace('motion::render_backend_guess', '::motion::render_backend_guess').replace('::::motion','::motion')
    # Both affine and mesh paths must preserve the fractional UV endpoints.
    render = render.replace('const std::array<int, 4> &', 'const std::array<double, 4> &')
    mesh_rect = 'tTVPRect(sourceRect[0], sourceRect[1],\n                         sourceRect[2], sourceRect[3])'
    assert render.count(mesh_rect) == 1
    render = render.replace(mesh_rect,
        'studysteady::motion::AtlasSampleRect{sourceRect[0], sourceRect[1],\n                         sourceRect[2], sourceRect[3]}')
    write_changed(OUT / 'GlesSceneItems.cpp', render)
    transform = extract((SOURCE / 'PlayerDrawDispatch.cpp').read_text(), 'bool Player::setDrawAffineTranslateMatrix(')
    write_changed(OUT / 'PlayerDrawTransform.cpp', '#include "PlayerInternal.h"\nnamespace motion {\n' + transform + '\n}\n')

if __name__ == '__main__': main()
