#pragma once
#include "mesh_combinator.h"
namespace motion { class Player; namespace detail { struct MotionNode; } }
namespace studysteady::motion {
void bindNodeMeshCombinator(::motion::detail::MotionNode &node,const tTJSVariant &layer);
// Raw, unnormalized E-mote variables. Updating local nodes marks true Player
// timeline state dirty so child deformation/geometry will consume the patch.
size_t updatePlayerCombinators(::motion::Player &player,const MeshVariables &variables);
void updatePlayerCombinatorsFromBindings(::motion::Player &player,::motion::Player &root);
void publishNodeMeshCombinator(::motion::detail::MotionNode &node);
}
