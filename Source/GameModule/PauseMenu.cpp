#include "PauseMenu.hpp"
#include "Common/Modules.hpp"
#include "GameModule/GameModule.hpp"
#include "GameModule/MusicFactory.hpp"
#include "InputModule/InputModule.hpp"
#include "SoundSystem/SoundSystem.hpp"
#include "UISystem/UIButton.hpp"
#include "UISystem/UIFactory.hpp"

PauseMenu::PauseMenu(sf::RenderWindow* renderWindow, const std::string& pauseFont, const std::string& pathToIniFile)
{
    setWindow(renderWindow);

    UIFactory::makeScreen(pathToIniFile, this, pauseFont);

    handleInput();

    for (auto& element : elements)
    {
        if (std::shared_ptr<UIButton> button = std::dynamic_pointer_cast<UIButton>(element.second))
        {
            button->onHover = [button]() { button->dropShadows(Colors::YELLOW, Colors::RED); };
            button->onClick = [button]() { button->dropShadows(Colors::RED, Colors::YELLOW); };

            if (element.first == Buttons::RESUME)
            {
                button->onRelease = [this]()
                {
                    Modules::Game->setIsPaused(false);
                    isPauseMenuOpen = false;
                };
            }
            else if (element.first == Buttons::MENU)
            {
                button->onRelease = []()
                {
                    Modules::Game->setIsPaused(false);
                    Modules::Game->setCurrentScreen(Screens::MAIN_MENU);
                    Modules::Sounds->playMusic(static_cast<int32_t>(AllMusic::Title));
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

bool PauseMenu::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Resized)
    {
        view.setSize(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));
        updateUIElementPositions();
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

void PauseMenu::handleInput()
{
    Modules::Input->LoadInputSettings("../../Data/Config/input_config.ini");

    pauseID = Modules::Input->GetActionID("Pause");
    if (pauseID < 0)
    {
        LOG("Failed to get Pause action ID.");
        return;
    }

    pauseHandle = Modules::Input->RegisterEvent(pauseID, std::bind(&PauseMenu::onPause, this, std::placeholders::_1));
    if (pauseHandle < 0)
    {
        LOG("Failed to register Pause event.");
        return;
    }
}

void PauseMenu::onPause(void* buttonState)
{
    bool state = *reinterpret_cast<bool*>(buttonState);
    if (state && !isPauseMenuOpen)
    {
        Modules::Game->setIsPaused(state);
        isPauseMenuOpen = state;
    }
    else if (state && isPauseMenuOpen)
    {
        Modules::Game->setIsPaused(!state);
        isPauseMenuOpen = !state;
    }
}
