#include "Common/Logs.hpp"
#include "GameModule/Bomb.hpp"
#include "GameModule/Booster.hpp"
#include "GameModule/EnemyBase.hpp"
#include "GameModule/Onli.hpp"
#include "GameModule/Gate.hpp"
#include "GameModule/PlayerCharacter.hpp"
#include "GameModule/UnbreakableObstacle.hpp"
#include "EnemyCollisionComponent.hpp"

bool EnemyCollisionComponent::BeginOverlapHandler(void* other)
{
    if (auto enemy = dynamic_cast<EnemyBase*>(getObjectParent()))
    {
        auto otherComponent = static_cast<CollisionComponent*>(other);
        if (auto player = dynamic_cast<PlayerCharacter*>(otherComponent->getObjectParent()))
        {
            player->die();
            return true;
        }
        else if (dynamic_cast<Obstacle*>(otherComponent->getObjectParent()) || 
            dynamic_cast<UnbreakableObstacle*>(otherComponent->getObjectParent()) ||
            dynamic_cast<Bomb*>(otherComponent->getObjectParent()))
        {
            if (auto onli = dynamic_cast<Onli*>(getObjectParent())) 
            {
                onli->handleObstacleOverlap();
            }
        }
    }
    return false;
}

void EnemyCollisionComponent::EndOverlapHandler(void*)
{
}
