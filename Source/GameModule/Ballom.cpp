#include "Ballom.hpp"
#include "BallomStates.hpp"
#include "Common/Modules.hpp"
#include "SpriteModule/Animation.hpp"
#include "SpriteModule/Sprite.hpp"
#include "SpriteModule/SpriteModule.hpp"

void Ballom::initialize(EnemyType type, sf::Vector2f spawnPosition)
{
    position  = spawnPosition;
    enemyType = type;

    m_rightAnimationId   = Modules::Sprite->createAnimation("../../Data/Config/BallomAnimationRight.ini");
    m_leftAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/BallomAnimationLeft.ini");
    m_deatAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/BallomDeathAnimation.ini");
    m_currentAnimationId = m_deatAnimationId;

    collisionBox = std::make_unique<EnemyCollisionComponent>();
    ai           = std::make_unique<AIController>();
    collisionBox->setParent(this);
    collisionBox->setObjectParent(this);
    collisionBox->setRectangleProperties(spawnPosition, {64.f, 64.f});
    ai->setParent(this);

    lookRightState = std::make_shared<LookRightState>();
    lookLeftState  = std::make_shared<LookLeftState>();

    std::random_device          rd;
    std::mt19937                gen(rd());
    std::bernoulli_distribution d(0.5);

    if (d(gen))
    {
        ai->fsm->SetInitialState(lookRightState);
    }
    else
    {
        ai->fsm->SetInitialState(lookLeftState);
    }

    ai->fsm->AddTransition(lookLeftState, std::make_shared<LeftToRightTransition>(lookRightState));
    ai->fsm->AddTransition(lookRightState, std::make_shared<RightToLeftTransition>(lookLeftState));
}