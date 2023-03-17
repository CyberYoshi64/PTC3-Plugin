#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"
#include "CTRPluginFramework/Menu/MenuFolder.hpp"
#include "CTRPluginFramework/Sound.hpp"

#include <cstring>

namespace CTRPluginFramework
{

    PluginMenuHome::PluginMenuHome(std::string &name, bool showNoteBottom) :

        _noteTB("", "", showNoteBottom ? IntRect(20, 46, 280, 124) : IntRect(40, 30, 320, 180)),
        _gameGuideBtn(Button::Sysfont | Button::Rounded, "Manual", IntRect(35, 95, 120, 30), Icon::DrawGuide),
        _searchBtn(Button::Sysfont | Button::Rounded, "Search", IntRect(160, 95, 120, 30), Icon::DrawSearch),
        _arBtn(Button::Sysfont | Button::Rounded, "Action Replay", showNoteBottom ? IntRect(35, 176, 120, 30) : IntRect(35, 130, 120, 30)),
        _toolsBtn(Button::Sysfont | Button::Rounded, "Options", showNoteBottom ? IntRect(160, 176, 120, 30) : IntRect(160, 130, 120, 30), Icon::DrawTools),

        _InfoBtn(Button::Icon | Button::Toggle, IntRect(60, 30, 25, 25), Icon::DrawInfo)

    {
        _root = _folder = new MenuFolderImpl(name);
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
        _showVersion = false;
        _versionPosX = 0;

        ShowNoteBottom = showNoteBottom;

        _mode = 0;

        _gameGuideBtn.Lock();

        // Disable Toggle buttons
        _InfoBtn.Disable();

        if (!Preferences::Settings.AllowActionReplay)
            _arBtn.Lock();
        if (!Preferences::Settings.AllowSearchEngine)
            _searchBtn.Lock();
    }

    bool PluginMenuHome::operator()(EventList& eventList, int& mode, Time& delta)
    {
        static Task top([](void *arg)
        {
            PluginMenuHome *home = reinterpret_cast<PluginMenuHome *>(arg);
            if (home->ShowNoteBottom) {
                Renderer::SetTarget(TOP);
                home->_RenderTop();
            }
            else {
                Renderer::SetTarget(TOP);
                if (home->_noteTB.IsOpen())
                    home->_noteTB.Draw();
                else
                    home->_RenderTop();
            }

            return (s32)0;

        }, this, Task::AppCores);

        _mode = mode;

        // Process events
        if (_noteTB.IsOpen() && !ShowNoteBottom)
        {
            for (size_t i = 0; i < eventList.size(); i++)
                if (_noteTB.ProcessEvent(eventList[i]) == false)
                {
                    _InfoBtn.SetState(false);
                    break;
                }
        }
        else
        {
            for (size_t i = 0; i < eventList.size(); i++)
                _ProcessEvent(eventList[i]);
        }

        if (_toolsBtn()) _toolsBtn_OnClick();
        if (_arBtn()) _actionReplayBtn_OnClick();

        if (!ShowNoteBottom) {
            if (_gameGuideBtn()) _gameGuideBtn_OnClick();
            if (_searchBtn()) _searchBtn_OnClick();
            if (_InfoBtn()) _InfoBtn_OnClick();
        }

        // Update UI
        _Update(delta);

        // Render top
        top.Start();

        // RenderBottom
        _RenderBottom();

        top.Wait();

        mode = _mode;

        return (Window::BottomWindow.MustClose());
    }

    void PluginMenuHome::Append(MenuItem* item) const
    {
        _folder->Append(item);
    }

    void PluginMenuHome::Refresh(void)
    {
        // If the currently selected folder is root
        // Nothing to do
        if (_folder->_container != nullptr)
        {
            // If current folder is hidden, close it
            while (!_folder->Flags.isVisible)
            {
                MenuFolderImpl *p = _folder->_Close(_selector);

                if (p)
                {
                    _folder = p;
                    if (_selector >= 1)
                        _selector--;
                }
                else
                    break;
            }
        }

        // Check for the validity of _selector range
        MenuFolderImpl *folder = _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            _selector = 0;

    }

#define IsUnselectableEntry(item) (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)
    static u32  SelectableEntryCount(MenuFolderImpl &folder)
    {
        u32 count = 0;

        for (u32 i = 0; i < folder.ItemsCount(); i++)
        {
            MenuItem *item = folder[i];

            if (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)
                continue;
            else
                count++;
        }
        return (count);
    }

    static void ScrollUp(int &selector, MenuFolderImpl &folder, int step)
    {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the begining
        if (selector == 0)
        {
            // Else select last item
            selector = folder.ItemsCount() - 1;
            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollUp(selector, folder, step);
            else
                SoundEngine::PlayMenuSound(SoundEngine::Event::CURSOR);
            return;
        }
        // Else go up
        selector -= step;
        if (selector < 0)
            selector = 0;
        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector]))
        {
            step = step > 1 ? step - 1 : 1;
            ScrollUp(selector, folder, step);
        }
        else
            SoundEngine::PlayMenuSound(SoundEngine::Event::CURSOR);
    }

    static void ScrollDown(int &selector, MenuFolderImpl &folder, int step)
    {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the end
        if (selector == static_cast<int>(folder.ItemsCount()) - 1)
        {
            // Else select first item
            selector = 0;
            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollDown(selector, folder, step);
            else
                SoundEngine::PlayMenuSound(SoundEngine::Event::CURSOR);
            return;
        }
        // Else go down
        selector += step;
        if (selector >= static_cast<int>(folder.ItemsCount()))
            selector = folder.ItemsCount() - 1;
        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector]))
        {
            step = step > 1 ? step - 1 : 1;
            ScrollDown(selector, folder, step);
        }
        else
            SoundEngine::PlayMenuSound(SoundEngine::Event::CURSOR);
    }

    //###########################################
    // Process Event
    //###########################################
    void PluginMenuHome::_ProcessEvent(Event& event)
    {
        static Clock fastScroll;
        static Clock inputClock;
        static MenuItem* last = nullptr;

        MenuFolderImpl* folder = _folder;
        MenuItem* item;

        switch (event.type)
        {
            case Event::KeyDown:
            {
                if (fastScroll.HasTimePassed(Milliseconds(800)) && inputClock.HasTimePassed(Milliseconds(100)))
                {
                    switch (event.key.code)
                    {
                            /*
                            ** Selector
                            **************/
                        case Key::CPadUp:
                        case Key::DPadUp:
                        {
                            ScrollUp(_selector, *folder, 1);
                            break;
                        }
                        case Key::CPadDown:
                        case Key::DPadDown:
                        {
                            ScrollDown(_selector, *folder, 1);
                            break;
                        }
                        case Key::CPadLeft:
                        case Key::DPadLeft:
                        {
                            ScrollUp(_selector, *folder, 4);
                            break;
                        }
                        case Key::CPadRight:
                        case Key::DPadRight:
                        {
                            ScrollDown(_selector, *folder, 4);
                            break;
                        }
                        default: break;
                    }
                    inputClock.Restart();
                }
                break;
            } // Event::KeyDown
            case Event::KeyPressed:
            {
                switch (event.key.code)
                {
                        /*
                        ** Selector
                        **************/
                    case Key::CPadUp:
                    case Key::DPadUp:
                    {
                        ScrollUp(_selector, *folder, 1);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadDown:
                    case Key::DPadDown:
                    {
                        ScrollDown(_selector, *folder, 1);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadLeft:
                    case Key::DPadLeft:
                    {
                        ScrollUp(_selector, *folder, 4);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadRight:
                    case Key::DPadRight:
                    {
                        ScrollDown(_selector, *folder, 4);
                        fastScroll.Restart();
                        break;
                    }
                        /*
                        ** Trigger entry
                        ** Top Screen
                        ******************/
                    case Key::A:
                    {
                        _TriggerEntry();
                        break;
                    }
                        /*
                        ** Closing Folder
                        ********************/
                    case Key::B:
                    {
                        MenuFolderImpl *newFolder = folder->_Close(_selector, false);

                        // Call the MenuEntry::OnAction callback if there's one
                        if (folder->_owner != nullptr && folder->_owner->OnAction != nullptr)
                            folder->_owner->OnAction(*_folder->_owner, ActionType::Closing);

                        // Switch current folder
                        if (newFolder != nullptr)
                        {
                            SoundEngine::PlayMenuSound(SoundEngine::Event::CANCEL);
                            _folder = newFolder;
                        }
                        break;
                    }
                    default: break;
                }
                break;
            } // End Key::Pressed event
            default: break;
        } // End switch

        folder = _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            _selector = 0;

        /*
        ** Scrolling text variables
        *********************************/
        if (folder->ItemsCount() > 0 && event.key.code != Key::Touchpad && (event.type < Event::TouchBegan || event.type > Event::TouchSwipped))
        {
            item = folder->_items[_selector];
            _selectedTextSize = Renderer::GetTextSize(item->name.c_str());
            _maxScrollOffset = static_cast<float>(_selectedTextSize) - 200.f;
            _scrollClock.Restart();
            _scrollOffset = 0.f;
            _reverseFlow = false;
        }
        else if (folder->ItemsCount() == 0)
        {
            _selectedTextSize = 0;
            _InfoBtn.Enable(false);
            _InfoBtn.SetState(false);
        }

        /*
        ** Update icon buttons state
        **************************/
        if (folder->ItemsCount() > 0 && _selector < static_cast<int>(folder->ItemsCount()))
        {
            item = folder->_items[_selector];

            if (last != item)
            {
                last = item;

                if (!ShowNoteBottom)
                {
                    // Toggle the info button
                    if (item->GetNote().size() > 0)
                    {
                        _noteTB.Update(item->name, item->GetNote());
                        _InfoBtn.Enable(true);
                    }
                    else
                        _InfoBtn.Enable(false);
                }
                else
                {
                    _noteTB.Update(item->firstName, item->GetNote());
                }
            }
        }
    }

    //###########################################
    // Render Menu
    //###########################################

    void PluginMenuHome::_RenderTop(void)
    {
        const Color &selected = Preferences::Settings.MenuSelectedItemColor;
        const Color &unselected = Preferences::Settings.MenuUnselectedItemColor;
        const Color &maintext = Preferences::Settings.MainTextColor;

        int posY = 25;
        int posX = 40;


        // Draw background
        Window::TopWindow.Draw();

        MenuFolderImpl* folder = _folder;

        // Draw Title
        int maxWidth = _showVersion ? _versionPosX - 10 : 360;
        int posYbak = posY;
        int width = Renderer::DrawSysString(folder->name.c_str(), posX, posY, maxWidth, maintext);
        Renderer::DrawLine(posX, posY, width, maintext);
        posY += 7;

        if (_showVersion && !folder->HasParent())
            Renderer::DrawSysString(_versionStr.c_str(), _versionPosX, posYbak, 360, maintext);

        // Draw Entry
        u32  drawSelector = SelectableEntryCount(*folder);
        int max = folder->ItemsCount();
        if (max == 0)
            return;
        int i = std::max(0, _selector - 6);
        max = std::min(max, (i + 8));

        for (; i < max; i++)
        {
            MenuItem    *item = folder->_items[i];
            ItemFlags   flags = item->Flags;
            const char  *name = item->name.c_str();
            const Color  &fg = i == _selector ? selected : unselected;
            float       offset = i == _selector ? _scrollOffset : 0.f;

            // Draw separator if needed
            if (flags.useSeparatorBefore)
            {
                if (flags.useStippledLineForBefore)
                    Renderer::DrawStippledLine(posX, posY - 1, 320, unselected, 1);
                else
                    Renderer::DrawLine(posX, posY - 1, 320, unselected, 1);
            }

            // Draw cursor
            if (drawSelector && i == _selector)
                Renderer::MenuSelector(posX - 5, posY - 3, 330, 20);

            // Draw entry
            if (item->_type == MenuType::Entry)
            {
                MenuEntryImpl   *entry = reinterpret_cast<MenuEntryImpl *>(item);

                if (entry->GameFunc != nullptr)
                    Renderer::DrawSysCheckBox(name, posX, posY, 350, fg, entry->IsActivated(), offset);
                else
                {
                    if (entry->MenuFunc != nullptr && !entry->_flags.isUnselectable)
                        Icon::DrawSettings(posX, posY);

                    Renderer::DrawSysString(name, posX + 20, posY, 350, fg, offset);
                    posY += 1;
                }
            }
            // Draw folder
            else
            {
                Renderer::DrawSysFolder(name, posX, posY, 350, fg, offset);
            }

            // Draw separator if needed
            if (flags.useSeparatorAfter)
            {
                if (flags.useStippledLineForAfter)
                    Renderer::DrawStippledLine(posX, posY - 1, 320, unselected, 1);
                else
                    Renderer::DrawLine(posX, posY - 1, 320, unselected, 1);
            }
            posY += 4;
        }
    }

    //###########################################
    // Render Bottom Screen
    //###########################################

    void PluginMenuHome::_RenderBottom(void)
    {
        Renderer::SetTarget(BOTTOM);

        Window::BottomWindow.Draw();

        // Draw buttons
        if (ShowNoteBottom)
        {
            _noteTB.Draw();
        }
        else
        {
            _gameGuideBtn.Draw();
            _searchBtn.Draw();
            _InfoBtn.Draw();
        }
        _arBtn.Draw();
        _toolsBtn.Draw();
    }

    //###########################################
    // Update menu
    //###########################################
    void PluginMenuHome::_Update(Time delta)
    {
        /*
        ** Scrolling
        *************/

        if (_selectedTextSize >= 280 && _scrollClock.HasTimePassed(Seconds(2)))
        {
            if (!_reverseFlow && _scrollOffset < _maxScrollOffset)
            {
                _scrollOffset += 29.f * delta.AsSeconds();
            }
            else
            {
                _scrollOffset -= 55.f * delta.AsSeconds();
                if (_scrollOffset <= 0.0f)
                {
                    _reverseFlow = false;
                    _scrollOffset = 0.f;
                    _scrollClock.Restart();
                }
                else
                    _reverseFlow = true;
            }
        }

        /*
        ** Buttons
        *************/

        // Buttons visibility

        MenuFolderImpl *folder = _folder;

        if (folder && (*folder)[_selector])
        {
            // If current folder is empty
            if (folder->ItemsCount() == 0 && !ShowNoteBottom)
            {
                _InfoBtn.Enable(false);
            }
            // If selected object is a folder
            else if ((*folder)[_selector]->IsFolder())
            {
                MenuFolderImpl *e = reinterpret_cast<MenuFolderImpl *>((*folder)[_selector]);

                if (!ShowNoteBottom)
                {
                    _InfoBtn.Enable(e->note.size());
                }
                if (e->HasNoteChanged())
                {
                    _noteTB.Update(e->firstName, e->GetNote());
                    e->HandledNoteChanges();
                }
            }

            // If selected object is an entry
            else if ((*folder)[_selector]->IsEntry())
            {
                MenuEntryImpl   *e = reinterpret_cast<MenuEntryImpl *>((*folder)[_selector]);
                std::string     &note = e->GetNote();

                if (!ShowNoteBottom)
                {
                    _InfoBtn.Enable(note.size());
                }
                if (e->HasNoteChanged())
                {
                    _noteTB.Update(e->firstName, note);
                    e->HandledNoteChanges();
                }
            }
            // An error is happening
            else
            {
                _InfoBtn.Enable(false);
            }
        }
        // Buttons status
        bool isTouched = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        if (!ShowNoteBottom)
        {
            // Update buttons
            _gameGuideBtn.Update(isTouched, touchPos);
            _searchBtn.Update(isTouched, touchPos);
            _InfoBtn.Update(isTouched, touchPos);
        }
        _arBtn.Update(isTouched, touchPos);
        _toolsBtn.Update(isTouched, touchPos);

        Window::BottomWindow.Update(isTouched, touchPos);
    }

    void PluginMenuHome::_TriggerEntry(void)
    {
        MenuFolderImpl* folder = _folder;


        if (_selector >= static_cast<int>(folder->ItemsCount()))
            return;

        MenuItem* item = folder->_items[_selector];

        /*
        ** MenuEntryImpl
        **************/
        if (item->_type == MenuType::Entry)
        {
            MenuEntryImpl* entry = reinterpret_cast<MenuEntryImpl *>(item);

            if (entry->_flags.isUnselectable)
            {
                SoundEngine::PlayMenuSound(SoundEngine::Event::CANCEL);
                return;
            }

            // If the entry has a valid funcpointer
            if (entry->GameFunc != nullptr)
            {
                // Change the state
                bool just = entry->_flags.justChanged;
                bool state = entry->_TriggerState();

                // If is activated add to executeLoop
                if (state)
                {
                    SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
                    PluginMenuExecuteLoop::Add(entry);
                }
                else if (just)
                {
                    SoundEngine::PlayMenuSound(SoundEngine::Event::DESELECT);
                    PluginMenuExecuteLoop::Remove(entry);
                }
            }
            else if (entry->MenuFunc != nullptr)
            {
                SoundEngine::PlayMenuSound(SoundEngine::Event::ACCEPT);
                entry->MenuFunc(entry->_owner);
            }
        }
        /*
        ** MenuFolderImpl
        ****************/
        else
        {
            MenuFolderImpl* p = reinterpret_cast<MenuFolderImpl *>(item);

            // If a MenuFolder exists and has a callback
            if (p->_owner != nullptr && p->_owner->OnAction != nullptr)
            {
                // If the callabck tells us to not open the folder
                if (!(p->_owner->OnAction(*p->_owner, ActionType::Opening)))
                {
                    SoundEngine::PlayMenuSound(SoundEngine::Event::CANCEL);
                    return;
                }
            }
            SoundEngine::PlayMenuSound(SoundEngine::Event::ACCEPT);
            p->_Open(folder, _selector, false);
            _folder = p;
            _selector = 0;
        }
    }

    void    PluginMenuHome::_controllerBtn_OnClick(void)
    {
        MenuFolderImpl* f = _folder;
        MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
        MenuEntry *entry = e->_owner;

        if (entry != nullptr)
        {
            if (entry->Hotkeys.Count() == 1)
            {
                entry->Hotkeys[0].AskForKeys();
                if (entry->Hotkeys._callback != nullptr)
                    entry->Hotkeys._callback(entry, 0);
                entry->RefreshNote();
            }
            else if (entry->Hotkeys.Count())
            {
                entry->Hotkeys.AskForKeys();
            }
        }
    }

    void    PluginMenuHome::_keyboardBtn_OnClick(void)
    {
        MenuFolderImpl *f = _folder;
        MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>((*f)[_selector]);

        if (e->MenuFunc != nullptr)
            e->MenuFunc(e->_owner);
    }

    void PluginMenuHome::_actionReplayBtn_OnClick()
    {
        _mode = 4;
    }

    void    PluginMenuHome::_gameGuideBtn_OnClick(void)
    {
        _mode = 2;
    }

    void    PluginMenuHome::_searchBtn_OnClick(void)
    {
        _mode = 3;
    }

    void    PluginMenuHome::_toolsBtn_OnClick(void)
    {
        _mode = 5;
    }

    void    PluginMenuHome::_InfoBtn_OnClick(void)
    {
        if (_noteTB.IsOpen())
            _noteTB.Close();
        else
            _noteTB.Open();
    }

    void    PluginMenuHome::Init(void)
    {
        MenuFolderImpl* folder = _folder;
        MenuItem    *item = folder->ItemsCount() != 0 ? folder->_items[0] : nullptr;

        // Init buttons state
        _InfoBtn.Enable(item != nullptr ? !item->GetNote().empty() : false);
    }

    void    PluginMenuHome::AddPluginVersion(u32 version)
    {
        char buffer[100];

        sprintf(buffer, "[%d.%d.%d]", static_cast<int>(version & 0xFF), static_cast<int>((version >> 8) & 0xFF), static_cast<int>(version >> 16));
        _versionStr.clear();
        _versionStr = buffer;

        float width = Renderer::GetTextSize(buffer);

        _versionPosX = 360 - (width + 1);
        _showVersion = true;
    }

    void    PluginMenuHome::Close(MenuFolderImpl *folder)
    {
        if (folder != _root)
        {
            if(_folder == folder)
                _folder = _folder->_Close(_selector, false);
        }
    }

    void    PluginMenuHome::UpdateNote(void)
    {
        if (!ShowNoteBottom)
            return;

        MenuFolderImpl* folder = _folder;

        if (!folder || !((*folder)[_selector]))
            return;

        if ((*folder)[_selector]->IsFolder())
        {
            MenuFolderImpl* e = reinterpret_cast<MenuFolderImpl*>((*folder)[_selector]);
            if (e->HasNoteChanged())
            {
                _noteTB.Update(e->firstName, e->GetNote());
                e->HandledNoteChanges();
            }
        }
        else if ((*folder)[_selector]->IsEntry())
        {
            MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl*>((*folder)[_selector]);
            if (e->HasNoteChanged())
            {
                _noteTB.Update(e->firstName, e->GetNote());
                e->HandledNoteChanges();
            }
        }
    }
}
