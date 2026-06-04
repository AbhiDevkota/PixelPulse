### How to Audio load work in Pixel Pulse

```text
Application Start
    ↓
FileManager::initialize() (in main)
    ↓
HomeScreen created (audio loaded with default 70.0f)
    ↓
HomeScreen::initialize() (NEW FLOW)
    ├→ FileManager::loadAllVolumeData() - reads saved volumes
    ├→ FileManager::getVolumeData() - retrieves "master_volume" and "effect_volume"
    ├→ Apply volumes to bgMusic_, selectSnd_, launchSnd_
    └→ Settings::initialize() - displays correct volume levels in UI
    ↓
User hears saved audio levels immediately!
```