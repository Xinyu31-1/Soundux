#include "hotkeys.hpp"
#include "../global/globals.hpp"
#include <cstdint>

namespace Soundux
{
    namespace traits
    {
        template <typename T> struct is_pair
        {
          private:
            static std::uint8_t test(...);
            template <typename O>
            static auto test(O *) -> decltype(std::declval<O>().first, std::declval<O>().second, std::uint16_t{});

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
    } // namespace traits
    namespace Objects
    {
        void Hotkeys::init()
        {
            listener = std::thread([this] { listen(); });
        }
        void Hotkeys::shouldNotify(bool status)
        {
            pressedKeys.clear();
            notify = status;
        }
        void Hotkeys::onKeyUp(int key)
        {
            if (notify && !pressedKeys.empty() &&
                std::find(pressedKeys.begin(), pressedKeys.end(), key) != pressedKeys.end())
            {
                Globals::gGui->onHotKeyReceived(pressedKeys);
                pressedKeys.clear();
            }
            else
            {
                pressedKeys.erase(std::remove_if(pressedKeys.begin(), pressedKeys.end(),
                                                 [key](const auto &item) { return key == item; }),
                                  pressedKeys.end());
            }
        }
        bool isCloseMatch(std::vector<int> pressedKeys, std::vector<int> keys)
        {
            std::sort(keys.begin(), keys.end());
            std::sort(pressedKeys.begin(), pressedKeys.end());

            if (pressedKeys.size() > keys.size())
            {
                std::size_t diff = pressedKeys.size() - keys.size();
                if (std::equal(pressedKeys.begin() + static_cast<int>(diff), pressedKeys.end(), keys.begin()))
                {
                    return true;
                }
            }
            return false;
        }
        template <typename T> std::optional<Sound> getBestMatch(const T &list, std::vector<int> pressedKeys)
        {
            std::optional<Sound> rtn;
            std::sort(pressedKeys.begin(), pressedKeys.end());

            for (const auto &_sound : list)
            {
                const auto &sound = [&] {
                    if constexpr (traits::is_pair<std::decay_t<decltype(_sound)>>::value)
                    {
                        return _sound.second.get();
                    }
                    else
                    {
                        return _sound;
                    }
                }();

                if (sound.hotkeys.empty())
                    continue;

                auto sortedKeys = sound.hotkeys;
                std::sort(sortedKeys.begin(), sortedKeys.end());

                if (sortedKeys == pressedKeys)
                {
                    rtn = sound;
                    break;
                }

                if (rtn && rtn->hotkeys.size() > sortedKeys.size())
                {
                    continue;
                }

                if (isCloseMatch(pressedKeys, sortedKeys))
                {
                    rtn = sound;
                }
            }
            return rtn;
        }
        void Hotkeys::onKeyDown(int key)
        {
            pressedKeys.emplace_back(key);

            if (notify)
            {
                return;
            }

            if (!Globals::gSettings.stopHotkey.empty() && (pressedKeys == Globals::gSettings.stopHotkey ||
                                                           isCloseMatch(pressedKeys, Globals::gSettings.stopHotkey)))
            {
                Globals::gGui->stopSounds();
                return;
            }

            std::optional<Sound> bestMatch;
            if (Globals::gSettings.tabHotkeysOnly)
            {
                auto tab = Globals::gData.getTab(Globals::gSettings.selectedTab);
                if (tab)
                {
                    bestMatch = getBestMatch(tab->sounds, pressedKeys);
                }
            }
            else
            {
                bestMatch = getBestMatch(Globals::gSounds, pressedKeys);
            }

            if (bestMatch)
            {
                auto pSound = Globals::gGui->playSound(bestMatch->id);
                if (pSound)
                {
                    Globals::gGui->onSoundPlayed(*pSound);
                }
            }
        }
        std::string Hotkeys::getKeySequence(const std::vector<int> &keys)
        {
            std::string rtn;
            for (const auto &key : keys)
            {
                rtn += getKeyName(key) + " + ";
            }
            if (!rtn.empty())
            {
                return rtn.substr(0, rtn.length() - 3);
            }
            return "";
        }
    } // namespace Objects
} // namespace Soundux