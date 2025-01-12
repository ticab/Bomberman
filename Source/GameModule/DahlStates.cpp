#pragma once
#include "DahlStates.hpp"
#include "CollisionModule/PhysicsModule.hpp"
#include "Common/FloatUtils.hpp"
#include "Common/Logs.hpp"
#include "Dahl.hpp"
#include "EnemyBase.hpp"
#include "LevelHandlingModule/LevelHandlingModule.hpp"
//#include "Common/Modules.hpp"
#include "LevelHandlingModule/Level.hpp"
#include "LevelHandlingModule/LevelHandlingModule.hpp"

namespace
{
const int32_t      RAYCAST_OFFSET        = 32;
const int32_t      PARENT_OFFSET         = 64;
const int32_t      TILE_SIZE             = 64;
const float        RAYCAST_LENGTH        = 2.f;
const int32_t      ANIMATION_CHANGE_TIME = 2;
const sf::Vector2f RIGHT                 = {1.f, 0.f};
const sf::Vector2f LEFT                  = {-1.f, 0.f};
} // namespace

/*
 * follow state 
 */
void FollowState::Enter(AIController* ai)
{
    ai->setIsPlayerInRange(false);

    auto parent                  = static_cast<Dahl*>(ai->getParent());
    m_startTime                  = std::chrono::steady_clock::now();
    sf::Vector2f collisionCenter = parent->getCollisionBox().getCenter();
    sf::Vector2i parentPosition2 = static_cast<sf::Vector2i>(collisionCenter) / TILE_SIZE;

    parent->playAnimation(Direction::Left);
    parent->m_navModule->algorithm->navigate({1, 7});
    parent->m_navModule->algorithm->moveTo({parentPosition2.y, parentPosition2.x}, m_moveTo);
    m_dir = {static_cast<float>(m_moveTo.x - parentPosition2.y), static_cast<float>(m_moveTo.y - parentPosition2.x)};
    m_currentDir = m_dir;

    switchAnimation(parent);
}

void FollowState::Update(AIController* ai)
{
    auto         parent          = static_cast<Dahl*>(ai->getParent());
    sf::Vector2f collisionCenter = parent->getCollisionBox().getCenter();
    sf::Vector2i parentPos       = {static_cast<int32_t>(collisionCenter.x - (m_dir.x * 32.f)) / TILE_SIZE,
                                    static_cast<int32_t>(collisionCenter.y - (m_dir.y * 32.f)) / TILE_SIZE};
    sf::Vector2i playerPos       = (sf::Vector2i(parent->getPlayerPos()) + sf::Vector2i(20, 20)) / TILE_SIZE;
    m_currentPos                 = parentPos;

    parent->m_navModule->algorithm->navigate({playerPos.y, playerPos.x});

    if (m_moveTo.x == m_currentPos.y && m_moveTo.y == m_currentPos.x)
    {
        parent->m_navModule->algorithm->moveTo({parentPos.y, parentPos.x}, m_moveTo);
    }

    m_dir = {static_cast<float>(m_moveTo.y - parentPos.x), static_cast<float>(m_moveTo.x - parentPos.y)};
    if (!parent->getIsDeathInitialized())
    {
        parent->setPosition({parent->getPosition().x + (m_dir.x * parent->getVelocity()),
                             parent->getPosition().y + (m_dir.y * parent->getVelocity())});
        parent->getCollisionBox().setRectangleProperties(parent->getPosition(), {64.f, 64.f});
    }


    if (!FloatUtils::areVectorsEqual(m_dir, m_currentDir))
    {
        switchAnimation(parent);
        m_currentDir = m_dir;
    }

    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();

    if (!parent->getIsDeathInitialized() && !FloatUtils::areVectorsEqual(m_dir, LEFT) &&
        !FloatUtils::areVectorsEqual(m_dir, RIGHT) && elapsed > ANIMATION_CHANGE_TIME)
    {
        m_startTime = std::chrono::steady_clock::now();
        switchAnimation(parent);
    }
}

void FollowState::Exit(AIController*)
{
    return;
}

void FollowState::switchAnimation(Dahl* parent)
{
    if (FloatUtils::areVectorsEqual(m_dir, RIGHT))
    {
        parent->playAnimation(Direction::Right);
    }
    else if (FloatUtils::areVectorsEqual(m_dir, LEFT) || !m_d(m_gen))
    {
        parent->playAnimation(Direction::Left);
    }
    else
    {
        parent->playAnimation(Direction::Right);
    }
}

/*
 * standBy state
 */
void StandbyState::Enter(AIController* ai)
{
    ai->setIsPlayerInRange(true);
}

void StandbyState::Update(AIController* ai)
{
    auto         parent          = static_cast<EnemyBase*>(ai->getParent());
    sf::Vector2f collisionCenter = parent->getCollisionBox().getCenter();
    sf::Vector2i parentPosition  = static_cast<sf::Vector2i>(parent->getPosition()) / TILE_SIZE;
    auto         now             = std::chrono::steady_clock::now();
    auto         elapsed         = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    if (elapsed > 2)
    {
        for (const auto& direction : directions)
        {
            sf::Vector2f endPoint;
            sf::Vector2f raycastStartingPos = {collisionCenter.x + (direction.x * RAYCAST_OFFSET),
                                               collisionCenter.y + (direction.y * RAYCAST_OFFSET)};
            bool isNotColiding = Modules::Physics->rayCast(raycastStartingPos, direction, RAYCAST_LENGTH, endPoint) == nullptr;
            isNotColiding &= Modules::Level->getTileInfo(parentPosition.x + static_cast<int32_t>(direction.x),
                                                         parentPosition.y + static_cast<int32_t>(direction.y)) ==
                             "Walkable";
            if (isNotColiding)
            {
                ai->setIsPlayerInRange(true);
                break;
            }
        }

        d(gen) ? parent->playAnimation(Direction::Left) : parent->playAnimation(Direction::Right);

        startTime = std::chrono::steady_clock::now();
    }
}

void StandbyState::Exit(AIController*)
{
    return;
}

/*
 * right to left state transition
 */
bool FollowToStandbyStateTransition::ShouldTrigger(AIController* ai) const
{
    return ai->getIsPlayerInRange();
}

std::shared_ptr<State> FollowToStandbyStateTransition::GetTargetState() const
{
    return targetState;
}

/*
 * left to right state transition
 */
bool StandbyToFollowStateTransition::ShouldTrigger(AIController* ai) const
{
    return ai->getIsPlayerInRange();
}

std::shared_ptr<State> StandbyToFollowStateTransition::GetTargetState() const
{
    return targetState;
}
