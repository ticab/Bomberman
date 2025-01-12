#include "Onli.hpp"
#include "Common/Modules.hpp"
#include "OnliStates.hpp"
#include "SpriteModule/Animation.hpp"
#include "SpriteModule/Sprite.hpp"
#include "SpriteModule/SpriteModule.hpp"

// can be deleted when finish
#include "Common/Logs.hpp"

void Onli::initialize(EnemyType type, sf::Vector2f spawnPosition)
{
    position  = spawnPosition;
    enemyType = type;

    m_rightAnimationId   = Modules::Sprite->createAnimation("../../Data/Config/OnliAnimationRight.ini");
    m_leftAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/OnliAnimationLeft.ini");
    m_deatAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/OnliDeathAnimation.ini");
    m_currentAnimationId = m_deatAnimationId;

    collisionBox = std::make_unique<EnemyCollisionComponent>();
    ai           = std::make_unique<AIController>();
    collisionBox->setParent(this);
    collisionBox->setObjectParent(this);
    collisionBox->setRectangleProperties(spawnPosition + sf::Vector2f(2.f, 2.f), {60.f, 60.f});
    ai->setParent(this);

    patrollingState = std::make_shared<PatrollingState>();
    restState       = std::make_shared<RestState>();

    ai->fsm->SetInitialState(restState);

    ai->fsm->AddTransition(patrollingState, std::make_shared<PatrollingToRestTransition>(restState));
    ai->fsm->AddTransition(restState, std::make_shared<RestToPatrollingTransition>(patrollingState));
}

void Onli::handleObstacleOverlap()
{
    patrollingState->handleObstacleOverlap(this);
}
