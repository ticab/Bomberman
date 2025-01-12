#pragma once
#include "CollisionComponent.hpp"

class EnemyCollisionComponent : public CollisionComponent
{
public:
    bool BeginOverlapHandler(void*) override;

    void EndOverlapHandler(void*) override;
};