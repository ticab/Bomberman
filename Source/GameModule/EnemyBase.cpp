#include "GameModule/EnemyBase.hpp"
#include "Common/Logs.hpp"
#include "Common/Modules.hpp"
#include "EventSystem/EventSystem.hpp"
#include "GameModule/MusicFactory.hpp"
#include "LevelHandlingModule/LevelHandlingModule.hpp"
#include "SoundSystem/SoundSystem.hpp"
#include "SpriteModule/Animation.hpp"
#include "SpriteModule/Sprite.hpp"
#include "SpriteModule/SpriteModule.hpp"
#include <random>
#include <set>
#include <utility>

EnemyType EnemyBase::getType() const
{
    return enemyType;
}

sf::Vector2f EnemyBase::getPosition() const
{
    return position;
}

std::shared_ptr<Animation> EnemyBase::getCurrentAnimation() const
{
    return Modules::Sprite->getAnimation(m_currentAnimationId);
}

void EnemyBase::initializeDeath()
{
    if (!isEnemyDead)
    {
        if (Modules::Level->getCurrentLevel()->getEnemies().size() - 1 == 0)
        {
            Modules::Sounds->playSound(static_cast<int32_t>(AllSounds::AllEnemiesDead));
        }

        Modules::Sprite->getAnimation(m_currentAnimationId)->Stop();
        m_currentAnimationId = m_deatAnimationId;
        Modules::Sprite->getAnimation(m_currentAnimationId)->Play();
        Modules::Sprite->getAnimation(m_currentAnimationId)->setPosition(position);
        isEnemyDead        = true;
        isDeathInitialized = true;
    }
}

void EnemyBase::playAnimation(Direction direction)
{
    if (!Modules::Sprite->getAnimation(m_deatAnimationId)->isPlaying())
    {
        Modules::Sprite->getAnimation(m_currentAnimationId)->Stop();

        m_currentAnimationId = (direction == Direction::Right) ? m_rightAnimationId : m_leftAnimationId;

        Modules::Sprite->getAnimation(m_currentAnimationId)->Play();
        Modules::Sprite->getAnimation(m_currentAnimationId)->setPosition(position);
    }
}

bool EnemyBase::isDead() const
{
    if (!Modules::Sprite->getAnimation(m_currentAnimationId)->isPlaying() && isEnemyDead)
    {
        Modules::Events->emit(m_enemyDeathID, nullptr);
        return true;
    }
    return false;
}

void EnemyBase::updateVelocity(float deltaTime)
{
    velocity = speed * deltaTime;
}

void EnemyBase::setCallbackID(EventID enemyDeathID)
{
    m_enemyDeathID = enemyDeathID;
}