#include "GameModule/GameModule.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Common/Modules.hpp"
#include "ConfigSystem/ConfigSystem.hpp"
#include "EnemyBase.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/EventTypes.hpp"
#include "GameOver.hpp"
#include "HUD.hpp"
#include "InputModule/InputModule.hpp"
#include "Leaderboard.hpp"
#include "MainMenu.hpp"
#include "MusicFactory.hpp"
#include "Options.hpp"
#include "PauseMenu.hpp"
#include "SoundSystem/SoundSystem.hpp"
#include "SpriteModule/SpriteModule.hpp"
#include "StageScreen.hpp"
#include "TestModule/TestModule.hpp"
#include <SFML/Graphics.hpp>
#include <chrono>

namespace
{
const std::string PATH_WINDOW_INFO = "../../Data/Config/windowInfo.ini";
const std::string PATH_HUD         = "../../Data/Config/HUD.ini";
const std::string PATH_MAIN_MENU   = "../../Data/Config/mainMenu.ini";
const std::string PATH_PAUSE_MENU  = "../../Data/Config/pauseMenu.ini";
const std::string PATH_STAGE       = "../../Data/Config/stageScreen.ini";
const std::string PATH_LEADERBOARD = "../../Data/Config/leaderboardScreen.ini";
const std::string PATH_OPTIONS     = "../../Data/Config/options.ini";
const std::string PATH_GAMEOVER    = "../../Data/Config/gameOver.ini";
const std::string PATH_MUSIC       = "../../Data/Config/music.ini";
const std::string PATH_SOUNDS      = "../../Data/Config/sounds.ini";
const std::string BASE_LEVEL       = "../../Data/Config/BaseLevelConfig.ini";
const std::string WINDOW           = "Window";
const std::string WIDTH            = "width";
const std::string HEIGHT           = "height";
const std::string TITLE            = "title";
const std::string FONT             = "font";
const std::string GAME_TIME        = "gameTime";
const std::string STAGE            = "stage";
const std::string LIVES            = "lives";
const std::string MAX_STAGE        = "maxStage";
} // namespace

using Time     = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<float, std::micro>;

bool GameModule::initialize()
{
    // Reading config file
    Modules::Config->addFile(PATH_WINDOW_INFO);
    Modules::Config->addFile(PATH_HUD);
    Modules::Config->addFile(PATH_MAIN_MENU);
    Modules::Config->addFile(PATH_STAGE);
    Modules::Config->addFile(PATH_LEADERBOARD);
    Modules::Config->addFile(PATH_PAUSE_MENU);
    Modules::Config->addFile(PATH_OPTIONS);
    Modules::Config->addFile(PATH_GAMEOVER);
    Modules::Config->addFile(PATH_MUSIC);
    Modules::Config->addFile(PATH_SOUNDS);
    const ConfigFile& windowInfo = Modules::Config->getFile(PATH_WINDOW_INFO);

    GAME_TIMER_FINISHED = Modules::Events->registerEvent();
    QUEST_FAILED        = Modules::Events->registerEvent();
    PLAYER_DESTROYED    = Modules::Events->registerEvent();
    OBJECTIVE_COMPLETED = Modules::Events->registerEvent();

    //load and set current base level
    currentLevel = Modules::Level->loadLevel(BASE_LEVEL);
    Modules::Level->setCurrentLevel(currentLevel);

    //set elements on level
    if (!Modules::Level->setUpElementsOnLevel(currentStage))
        return false;

    if (!windowInfo.isSectionPresent(WINDOW))
        return false;

    const auto& windowSection = windowInfo.getSection(WINDOW);

    if (!windowSection.areValuesPresent({WIDTH, HEIGHT, FONT, TITLE, GAME_TIME, STAGE}))
        return false;

    int32_t width           = windowSection.getValue(WIDTH).getInt32();
    int32_t height          = windowSection.getValue(HEIGHT).getInt32();
    gameTitle               = windowSection.getValue(TITLE).getString();
    const std::string& font = windowSection.getValue(FONT).getString();
    gameTime                = windowSection.getValue(GAME_TIME).getInt32();
    maxStage                = windowSection.getValue(MAX_STAGE).getInt32();
    startingTime            = gameTime;
    currentStage            = windowSection.getValue(STAGE).getInt32();
    livesLeft               = currentStage + 1;

    // Creating Window and HUD
    window.create(sf::VideoMode(width, height), gameTitle, sf::Style::Close);
    Modules::UI->setViewportSize((float)width, (float)height);
    // Creating all screens
    screens[Screens::LEVEL]       = std::make_shared<HUD>(&window, font, PATH_HUD);
    screens[Screens::MAIN_MENU]   = std::make_shared<MainMenu>(&window, font, PATH_MAIN_MENU);
    screens[Screens::STAGE]       = std::make_shared<StageScreen>(&window, font, PATH_STAGE);
    screens[Screens::LEADERBOARD] = std::make_shared<Leaderboard>(&window, font, PATH_LEADERBOARD);
    screens[Screens::PAUSE_MENU]  = std::make_shared<PauseMenu>(&window, font, PATH_PAUSE_MENU);
    screens[Screens::OPTIONS]     = std::make_shared<Options>(&window, font, PATH_OPTIONS);
    screens[Screens::GAME_OVER]   = std::make_shared<GameOver>(&window, font, PATH_GAMEOVER);

    auto screenStage = (std::dynamic_pointer_cast<StageScreen>(screens[Screens::STAGE]));
    screenStage->setStage(currentStage);

    auto hudScreen = (std::dynamic_pointer_cast<HUD>(screens[Screens::LEVEL]));
    hudScreen->setTime(std::to_string(gameTime));
    hudScreen->setLivesLeft(std::to_string(livesLeft));

    MusicFactory::loadAllMusic(PATH_MUSIC);
    MusicFactory::loadAllSounds(PATH_SOUNDS);

    if (!player.init())
    {
        LOG("Failed to initialize PlayerCharacter.");
        return false;
    }

    gameStats = std::make_unique<GameStats>();
    gameStats->initialize(Modules::Level->getCurrentLevel());

    Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Title));

    freezeSound = AllSounds::Miss;
    return true;
}

void GameModule::run()
{
#ifndef FINAL
    Modules::Tests->run();
#endif

    Time::time_point currentTime;
    Time::time_point prevTime  = Time::now();
    float            deltaTime = 0.0f;

    while (window.isOpen())
    {
        // handling delta time
        currentTime = Time::now();
        deltaTime   = std::chrono::duration_cast<Duration>(currentTime - prevTime).count();
        prevTime    = currentTime;

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    saveResults();
                    window.close();
                    break;

                case sf::Event::Resized:
                {
                    window.setView(sf::View(sf::FloatRect(0.f, 0.f, (float)window.getSize().x, (float)window.getSize().y)));
                    Modules::UI->setViewportSize((float)(window.getSize().x), (float)(window.getSize().y));
                    for (auto& screen : screens)
                    {
                        screen.second->handleEvent(event);
                    }
                    break;
                }
                case sf::Event::MouseMoved:
                case sf::Event::MouseButtonPressed:
                case sf::Event::MouseButtonReleased:
                case sf::Event::TextEntered:
                case sf::Event::KeyReleased:
                    if (isPaused && currentScreen != Screens::OPTIONS)
                        screens[Screens::PAUSE_MENU]->handleEvent(event);
                    else
                        screens[currentScreen]->handleEvent(event);
                    break;
            }
        }
        if (!isPaused)
        {
            timeCounter += deltaTime;
            checkTimeCounter();
        }

#ifndef FINAL
        Modules::Tests->update(deltaTime, &window);
#endif
        updateBoosters();
        window.clear(screens[currentScreen]->getBackgroundColor());
        if (currentScreen == Screens::LEVEL)
        {
            sf::View tempView = screens[currentScreen]->getWindow()->getView();
            Modules::update(deltaTime, &window);
            Modules::Level->setLevelViewOffset(player.getCurrentPosition(), *screens[currentScreen]->getWindow());

            player.updateVelocity(deltaTime);

            Modules::Physics->updateCollision();

            player.updateBombs(deltaTime);
            player.drawBombs(window);

            window.draw(*player.getCurrentAnimation());
            player.setIsUpdated(false);

            Modules::Physics->updateCollision();

            for (const auto& enemyPtr : Modules::Level->getCurrentLevelPtr()->getEnemies())
            {
                // Cast to the derived type (e.g., Enemy)
                auto specificEnemyPtr = std::dynamic_pointer_cast<EnemyBase>(enemyPtr);

                if (specificEnemyPtr)
                {
                    // Now you can safely call 'getPlayerPos()' from 'Enemy'
                    specificEnemyPtr->setPlayerPosition(player.getCurrentPosition());
                }
            }

            screens[currentScreen]->getWindow()->setView(tempView);
            std::static_pointer_cast<HUD>(screens[Screens::LEVEL])->setScore(std::to_string(gameStats->getPoints()));

            screens[currentScreen]->getWindow()->setView(tempView);


            if (isPaused && !isFreezed)
            {
                screens[Screens::PAUSE_MENU]->draw(window, sf::RenderStates::Default);
            }
            else if (isFreezed && !Modules::Sounds->isSoundPlaying(static_cast<int32_t>(freezeSound)))
            {
                isFreezed = false;
                isPaused  = false;
                if (freezeSound == AllSounds::Miss)
                    playerDied();
                else if (freezeSound == AllSounds::Gate)
                    nextLevel();
            }
        }
        screens[currentScreen]->draw(window, sf::RenderStates::Default);
        window.display();
    }
}

void GameModule::terminate()
{
    gameStats->terminate();
}

void GameModule::setCurrentScreen(const Screens& newScreen)
{
    Modules::Sounds->pauseMusic();
    currentScreen = newScreen;
    timeCounter   = 0.0f;
}

void GameModule::handleResize(float x, float y)
{
    window.setView(sf::View(sf::FloatRect(0.f, 0.f, x, y)));
    Modules::UI->setViewportSize(x, y);

    // The only thing handleEvent needs is event type to be Resized
    sf::Event resizeEvent;
    resizeEvent.type = sf::Event::Resized;
    for (auto& screen : screens)
    {
        screen.second->handleEvent(resizeEvent);
    }
}

void GameModule::checkTimeCounter()
{
    switch (currentScreen)
    {
        case Screens::LEVEL:
            // Checking if one second has passed for updating Time label (delta time is in microseconds)
            if (timeCounter >= 1000000)
            {
                timeCounter = 0.0f;
                gameTime--;

                auto hud = (std::dynamic_pointer_cast<HUD>(screens[currentScreen]));
                hud->setTime(std::to_string(gameTime));
            }
            if (gameTime < 0)
            {
                playerDied();
            }
            break;
        case Screens::STAGE:
            // Checking if 3 seconds has passed for updating Screen
            if (timeCounter >= 3000000)
            {
                setCurrentScreen(Screens::LEVEL);

                Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Background));
            }
            break;
    }
}

void GameModule::updateBoosters()
{
    auto boostersIterator = m_boosters.begin();
    while (boostersIterator != m_boosters.end())
    {
        if ((*boostersIterator)->shouldRemoveEffect())
        {
            if ((*boostersIterator)->removeEffect(player))
            {
                boostersIterator = m_boosters.erase(boostersIterator);
                continue;
            }
        }
        ++boostersIterator;
    }
}

void GameModule::addScore(int32_t newScore, const std::string& name)
{
    (std::dynamic_pointer_cast<Leaderboard>(screens[Screens::LEADERBOARD]))->addScore(newScore, name);
}

void GameModule::saveResults()
{
    (std::dynamic_pointer_cast<Leaderboard>(screens[Screens::LEADERBOARD]))->saveResults();
}

float GameModule::getHUDHeight()
{
    auto hud = (std::dynamic_pointer_cast<HUD>(screens[Screens::LEVEL]));
    return hud->getBackgroundHeigth();
}

void GameModule::addBooster(std::shared_ptr<BoosterComponent> newBooster)
{
    m_boosters.push_back(newBooster);

    newBooster->applyEffect(player);
}

void GameModule::playerDied()
{
    if (--livesLeft >= 0)
    {
        auto hud = (std::dynamic_pointer_cast<HUD>(screens[Screens::LEVEL]));

        hud->setLivesLeft(std::to_string(livesLeft));
        gameTime = startingTime;
        hud->setTime(std::to_string(gameTime));

        player.resetToStart();

        Modules::Level->unloadLevel(currentLevel);
        Modules::Level->setUpElementsOnLevel(currentStage);

        setCurrentScreen(Screens::STAGE);
        Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Stage));
        //generate new level, same difficulty
    }
    else
    {
        (std::dynamic_pointer_cast<GameOver>(screens[Screens::GAME_OVER]))->setScore(gameStats->getPoints());
        setCurrentScreen(Screens::GAME_OVER);
        Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::GameOver));
    }
}

void GameModule::nextLevel()
{
    if (++currentStage > maxStage)
    {
        (std::dynamic_pointer_cast<GameOver>(screens[Screens::GAME_OVER]))->setScore(gameStats->getPoints());
        setCurrentScreen(Screens::GAME_OVER);
        Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::GameOver));
        return;
    }
    gameTime = startingTime;

    livesLeft = currentStage + 1;

    auto hud = (std::dynamic_pointer_cast<HUD>(screens[Screens::LEVEL]));
    hud->setLivesLeft(std::to_string(livesLeft));
    hud->setTime(std::to_string(gameTime));

    Modules::Level->unloadLevel(currentLevel);
    Modules::Level->setUpElementsOnLevel(currentStage);

    player.resetToStart();

    (std::dynamic_pointer_cast<StageScreen>(screens[Screens::STAGE]))->setStage(currentStage);
    setCurrentScreen(Screens::STAGE);
    Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Stage));
}

void GameModule::freeze(AllSounds sound)
{
    Modules::Sounds->stopMusic();
    Modules::Sounds->playSound(static_cast<int32_t>(sound));
    isFreezed   = true;
    isPaused    = true;
    freezeSound = sound;
}

void GameModule::removeAllBoosters()
{
    m_boosters.clear();
}
