#include "PlayerCollisionComponent.hpp"
#include "Common/Logs.hpp"
#include "GameModule/Bomb.hpp"
#include "GameModule/Booster.hpp"
#include "GameModule/EnemyBase.hpp"
#include "GameModule/Gate.hpp"
#include "GameModule/PlayerCharacter.hpp"
#include "GameModule/UnbreakableObstacle.hpp"

bool PlayerCollisionComponent::BeginOverlapHandler(void* other)
{
    if (auto player = dynamic_cast<PlayerCharacter*>(getObjectParent()))
    {
        auto otherComponent = static_cast<CollisionComponent*>(other);
        if (auto enemy = dynamic_cast<EnemyBase*>(otherComponent->getObjectParent()))
        {
            player->die();
            return true;
        }
        else if (dynamic_cast<Obstacle*>(otherComponent->getObjectParent()))
        {
            if (!player->getPassThroughWall())
            {
                player->handleObstacleOverlap();
            }
        }
        else if (dynamic_cast<UnbreakableObstacle*>(otherComponent->getObjectParent()))
        {
            player->handleObstacleOverlap();
        }
        else if (dynamic_cast<Bomb*>(otherComponent->getObjectParent()))
        {
            if (!player->getPassThroughBombs())
            {
                player->handleObstacleOverlap();
            }
        }
        else if (auto booster = dynamic_cast<Booster*>(otherComponent->getObjectParent()))
        {
            player->handleBoosterOverlap(booster);
        }
        else if (dynamic_cast<Gate*>(otherComponent->getObjectParent()))
        {
            if (player->handleGateOverlap())
                return true;
        }
    }
    return false;
}

void PlayerCollisionComponent::EndOverlapHandler(void*)
{
}
