
#include "includes.hpp"
#include "ui/record_layer.hpp"
#include "ui/game_ui.hpp"
#include "ui/clickbot_layer.hpp"
#include "ui/macro_editor.hpp"
#include "ui/render_settings_layer.hpp"
#include "hacks/layout_mode.hpp"
#include "hacks/show_trajectory.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

class $modify(CCKeyboardDispatcher) {
  $override
  bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {

    auto& g = Global::get();

    int keyInt = static_cast<int>(key);
    if (g.allKeybinds.contains(keyInt) && !isKeyRepeat) {
      for (size_t i = 0; i < 6; i++) {
        if (std::find(g.keybinds[i].begin(), g.keybinds[i].end(), keyInt) != g.keybinds[i].end())
          g.heldButtons[i] = isKeyDown;
      }
    }

    return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
  }
};

static void onKeybindAction(bool down, const std::string& id) {
  auto& g = Global::get();

  if (!down || (LevelEditorLayer::get() && !g.mod->getSettingValue<bool>("editor_keybinds")) || g.mod->getSettingValue<bool>("disable_keybinds"))
    return;

  if (g.state != state::recording && g.mod->getSettingValue<bool>("recording_only_keybinds"))
    return;

  if (id == "open-menu") {
    if (g.layer) {
      static_cast<RecordLayer*>(g.layer)->onClose(nullptr);
      return;
    }

    RecordLayer::openMenu();
  }

  if (id == "toggle-recording")
    Macro::toggleRecording();

  if (id == "toggle-playing")
    Macro::togglePlaying();

  if (id == "toggle-frame-stepper" && PlayLayer::get())
    Global::toggleFrameStepper();

  if (id == "step-frame")
    Global::frameStep();

  if (id == "toggle-speedhack")
    Global::toggleSpeedhack();

  if (id == "show-trajectory") {
    g.mod->setSavedValue("macro_show_trajectory", !g.mod->getSavedValue<bool>("macro_show_trajectory"));

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->trajectoryToggle)
        static_cast<RecordLayer*>(g.layer)->trajectoryToggle->toggle(g.mod->getSavedValue<bool>("macro_show_trajectory"));
    }

    g.showTrajectory = g.mod->getSavedValue<bool>("macro_show_trajectory");
    if (!g.showTrajectory) ShowTrajectory::trajectoryOff();
  }

  if (id == "toggle-render" && PlayLayer::get()) {
    bool result = Renderer::toggle();

    if (result && Global::get().renderer.recording)
      Notification::create("Started Rendering", NotificationIcon::Info)->show();

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->renderToggle)
        static_cast<RecordLayer*>(g.layer)->renderToggle->toggle(Global::get().renderer.recording);
    }

  }

  if (id == "toggle-noclip") {
    g.mod->setSavedValue("macro_noclip", !g.mod->getSavedValue<bool>("macro_noclip"));

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->noclipToggle)
        static_cast<RecordLayer*>(g.layer)->noclipToggle->toggle(g.mod->getSavedValue<bool>("macro_noclip"));
    }
  }
}

$on_mod(Loaded) {
  const std::vector<std::string> keybindIDs = {
    "open-menu", "toggle-recording", "toggle-playing",
    "toggle-speedhack", "toggle-frame-stepper", "step-frame",
    "toggle-render", "toggle-noclip", "show-trajectory"
  };

  for (const auto& id : keybindIDs) {
    listenForKeybindSettingPresses(id, [id](auto&, bool down, bool, double) {
      onKeybindAction(down, id);
    });
  }
}
