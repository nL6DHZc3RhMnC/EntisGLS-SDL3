#include "extensions/emote/tjs_runtime/player_mesh_bridge.h"
#include "Player.h"
#include "MotionDispatch.h"
#include <stdexcept>

namespace studysteady::motion {
void bindNodeMeshCombinator(::motion::detail::MotionNode &node,const tTJSVariant &layer) {
    using namespace ::motion::detail;
    const auto value=motionPropGet(layer,TJS_W("meshCombinator"));
    if(value.Type()==tvtVoid)return;
    if(node.meshType!=1)throw std::runtime_error("meshCombinator: node meshTransform must be 1");
    // All 78 haz_a combinators have null timeline bp/cc. The old and new
    // payload precedence for simultaneous nonempty sources is not verified.
    // Reject that case rather than silently discard either source.
    const auto frames=motionPropGet(layer,TJS_W("frameList"));
    for(int i=0;i<motionPropGetCount(frames);++i) {
        const auto content=motionPropGet(motionPropGetByNum(frames,i),TJS_W("content"));
        if(content.Type()==tvtVoid)continue;
        const auto mesh=motionPropGet(content,TJS_W("mesh"));
        if(mesh.Type()==tvtVoid)continue;
        if(motionPropGet(mesh,TJS_W("bp")).Type()!=tvtVoid || motionPropGet(mesh,TJS_W("cc")).Type()!=tvtVoid)
            throw std::runtime_error("meshCombinator with nonempty timeline mesh is not yet supported");
    }
    node.meshCombinator=std::make_shared<MeshCombinator>(value);
    node.meshCombinator->update({});
}
size_t updatePlayerCombinators(::motion::Player &player,const MeshVariables &variables) {
    size_t changed=0;
    for(auto &node:player.nodesForBuild()) {
        if(node.meshCombinator && node.meshCombinator->update(variables)) {
            node.flags|=1;node.accumulated.dirty=true;++changed;
        }
    }
    return changed;
}
void updatePlayerCombinatorsFromBindings(::motion::Player &player,::motion::Player &root) {
    MeshVariables values;
    for(const auto &node:player.nodes())if(node.meshCombinator)
        for(const auto &axis:node.meshCombinator->axes())
            values[axis.key]=static_cast<float>(root.getVariable(ttstr(axis.key)));
    updatePlayerCombinators(player,values);
}
void publishNodeMeshCombinator(::motion::detail::MotionNode &node) {
    if(node.meshCombinator)node.meshCombinator->publish(node.meshControlPoints);
}
}
