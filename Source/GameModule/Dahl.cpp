#pragma once
#include "Dahl.hpp"
#include "DahlStates.hpp"
#include "LevelHandlingModule/Level.hpp"
#include "LevelHandlingModule/LevelHandlingModule.hpp"

void Dahl::initialize(EnemyType type, sf::Vector2f spawnPosition)
{
    position  = spawnPosition;
    enemyType = type;

    m_rightAnimationId   = Modules::Sprite->createAnimation("../../Data/Config/DahlAnimationRight.ini");
    m_leftAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/DahlAnimationLeft.ini");
    m_deatAnimationId    = Modules::Sprite->createAnimation("../../Data/Config/DahlDeathAnimation.ini");
    m_currentAnimationId = m_deatAnimationId;

    collisionBox = std::make_unique<EnemyCollisionComponent>();
    ai           = std::make_unique<AIController>();
    collisionBox->setParent(this);
    collisionBox->setObjectParent(this);
    collisionBox->setRectangleProperties(spawnPosition, {64.f, 64.f});
    ai->setParent(this);

    m_folowState   = std::make_shared<FollowState>();
    m_standbyState = std::make_shared<StandbyState>();

    ai->fsm->SetInitialState(m_standbyState);

    ai->fsm->AddTransition(m_folowState, std::make_shared<FollowToStandbyStateTransition>(m_standbyState));
    ai->fsm->AddTransition(m_standbyState, std::make_shared<StandbyToFollowStateTransition>(m_folowState));
}

void Dahl::initialzieNavModule(std::shared_ptr<std::vector<std::vector<bool>>> navGrid)
{
    m_navModule = std::make_unique<NavigationModule>(navGrid);
}