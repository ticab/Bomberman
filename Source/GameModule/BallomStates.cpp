#include "BallomStates.hpp"
#include "Common/Logs.hpp"
#include "EnemyBase.hpp"

/*
 * look right state 
 */
void LookRightState::Enter(AIController* ai)
{
    auto enemy = static_cast<EnemyBase*>(ai->getParent());
    enemy->playAnimation(Direction::Right);
    startTime = std::chrono::steady_clock::now();
    ai->setIsPlayerInRange(false);
}

void LookRightState::Update(AIController* ai)
{
    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    if (elapsed >= 5)
    {
        if (d(gen))
        {
            ai->setIsPlayerInRange(true);
        }
        else
        {
            startTime = std::chrono::steady_clock::now();
        }
    }
}

void LookRightState::Exit(AIController*)
{
    return;
}

/*
 * look left state 
 */
void LookLeftState::Enter(AIController* ai)
{
    auto enemy = static_cast<EnemyBase*>(ai->getParent());
    enemy->playAnimation(Direction::Left);
    startTime = std::chrono::steady_clock::now();
    ai->setIsPlayerInRange(false);
}

void LookLeftState::Update(AIController* ai)
{
    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    if (elapsed >= 5)
    {
        if (d(gen))
        {
            ai->setIsPlayerInRange(true);
        }
        else
        {
            startTime = std::chrono::steady_clock::now();
        }
    }
}

void LookLeftState::Exit(AIController*)
{
    return;
}

/*
 * right to left state transition
 */
bool RightToLeftTransition::ShouldTrigger(AIController* ai) const
{
    return ai->getIsPlayerInRange();
}

std::shared_ptr<State> RightToLeftTransition::GetTargetState() const
{
    return targetState;
}

/*
 * left to right state transition
 */
bool LeftToRightTransition::ShouldTrigger(AIController* ai) const
{
    return ai->getIsPlayerInRange();
}

std::shared_ptr<State> LeftToRightTransition::GetTargetState() const
{
    return targetState;
}
