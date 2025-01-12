#include "MainMenu.hpp"
#include "Common/Logs.hpp"
#include "GameModule.hpp"
#include "MusicFactory.hpp"
#include "SoundSystem/SoundSystem.hpp"
#include "UIConstants.hpp"
#include "UISystem/UIButton.hpp"
#include "UISystem/UIFactory.hpp"

MainMenu::MainMenu(sf::RenderWindow* renderWindow, const std::string& font, const std::string& pathToIniFile)
    : UIScreen()
{
    setWindow(renderWindow);

    UIFactory::makeScreen(pathToIniFile, this, font);

    for (auto& element : elements)
    {
        if (std::shared_ptr<UIButton> button = std::dynamic_pointer_cast<UIButton>(element.second))
        {
            button->onHover = [button]() { button->dropShadows(Colors::YELLOW, Colors::RED); };
            button->onClick = [button]() { button->dropShadows(Colors::RED, Colors::YELLOW); };
            if (element.first == Buttons::START)
            {
                button->onRelease = []()
                {
                    Modules::Game->setCurrentScreen(Screens::STAGE);
                    Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Stage));
                };
            }
            else if (element.first == Buttons::LEADERBOARD)
            {
                button->onRelease = []()
                {
                    Modules::Game->setCurrentScreen(Screens::LEADERBOARD);
                    Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Leaderboard));
                };
            }
            else if (element.first == Buttons::OPTIONS)
            {
                button->onRelease = []() 
                { 
                    Modules::Game->setCurrentScreen(Screens::OPTIONS); 
                    Modules::Sounds->continueMusic();
                };
            }
        }
    }
}

bool MainMenu::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Resized)
    {
        return UIScreen::handleEvent(event);
    }
    for (auto& element : elements)
    {
        if (std::shared_ptr<UIButton> button = std::dynamic_pointer_cast<UIButton>(element.second))
        {
            if (!button->handleEvent(event))
            {
                button->dropShadows(Colors::WHITE, Colors::GREY);
            }
        }
    }
    return true;
}
