#include "input/InputAction.hpp"
#include "input/InputBinding.hpp"
#include "input/InputContext.hpp"
#include "input/InputBuffer.hpp"
#include "input/GamepadInput.hpp"
#include "input/InputSettings.hpp"
#include "input/InputManager.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/Camera2D.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>

using namespace wildbound;
using namespace wildbound::input;

void TestInputActions() {
    std::cout << "[TEST] 1. Input Actions & Enum Mapping..." << std::endl;

    assert(ActionToString(InputAction::MoveUp) == "MoveUp");
    assert(ActionToString(InputAction::PrimaryAttack) == "PrimaryAttack");
    assert(ActionToString(InputAction::OpenInventory) == "OpenInventory");
    assert(StringToAction("MoveUp") == InputAction::MoveUp);
    assert(StringToAction("PrimaryAttack") == InputAction::PrimaryAttack);
    assert(StringToAction("NonExistentAction") == InputAction::Count);

    assert(GetActionCategory(InputAction::MoveUp) == ActionCategory::Movement);
    assert(GetActionCategory(InputAction::PrimaryAttack) == ActionCategory::Combat);
    assert(GetActionCategory(InputAction::OpenInventory) == ActionCategory::Inventory);
    assert(GetActionCategory(InputAction::UIConfirm) == ActionCategory::UI);

    std::cout << "  -> Input Actions PASSED." << std::endl;
}

void TestInputBindings() {
    std::cout << "[TEST] 2. Input Bindings & Display Formatting..." << std::endl;

    InputBinding b1{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, KEY_W };
    assert(b1.IsValid());
    assert(BindingToString(b1) == "W");

    InputBinding b2{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, KEY_TAB, 0, 0, 0.5f, Mod_Shift };
    assert(BindingToString(b2) == "Shift+Tab");

    InputBinding b3{ DeviceType::Mouse, BindingSourceType::ButtonOrKey, MOUSE_BUTTON_LEFT };
    assert(BindingToString(b3) == "MouseLeft");

    InputBinding b4{ DeviceType::Gamepad, BindingSourceType::ButtonOrKey, GAMEPAD_BUTTON_RIGHT_FACE_DOWN };
    assert(BindingToString(b4) == "A / Cross");

    InputBinding b5{ DeviceType::Gamepad, BindingSourceType::Axis, 0, GAMEPAD_AXIS_LEFT_Y, -1 };
    assert(BindingToString(b5) == "LS_Y-");

    std::cout << "  -> Input Bindings PASSED." << std::endl;
}

void TestGamepadProcessing() {
    std::cout << "[TEST] 3. Gamepad Stick Radial Deadzone & Simulated Input..." << std::endl;

    GamepadInput gp;
    gp.SetDeadzone(0.20f);
    assert(std::abs(gp.GetDeadzone() - 0.20f) < 0.001f);

    // Test inside deadzone: length 0.15 <= 0.20 -> result must be exactly (0, 0)
    Vector2 insideDZ{ 0.09f, 0.12f }; // Length = sqrt(0.0081 + 0.0144) = sqrt(0.0225) = 0.15
    Vector2 processedInside = gp.ProcessStickDeadzone(insideDZ);
    assert(processedInside.x == 0.0f && processedInside.y == 0.0f);

    // Test outside deadzone: length 1.0 -> result length must be 1.0
    Vector2 fullStick{ 0.0f, -1.0f };
    Vector2 processedFull = gp.ProcessStickDeadzone(fullStick);
    assert(processedFull.x == 0.0f && std::abs(processedFull.y - (-1.0f)) < 0.001f);

    // Test smooth remapping from deadzone to 1.0
    Vector2 midStick{ 0.60f, 0.0f }; // Length = 0.60. Remapped: (0.60 - 0.20) / (1.0 - 0.20) = 0.40 / 0.80 = 0.50
    Vector2 processedMid = gp.ProcessStickDeadzone(midStick);
    assert(std::abs(processedMid.x - 0.50f) < 0.001f && processedMid.y == 0.0f);

    // Test simulated controller connection
    gp.SimulateConnection(0, true, "Xbox Wireless Controller");
    assert(gp.IsConnected(0));
    assert(gp.GetName(0) == "Xbox Wireless Controller");

    // Test simulated buttons
    gp.SimulateButton(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, true);
    gp.Update(0);
    assert(gp.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0));
    assert(gp.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0));

    // Next frame button still held
    gp.Update(0);
    assert(gp.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0));
    assert(!gp.IsButtonPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0)); // Pressed only on initial transition

    // Button released
    gp.SimulateButton(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, false);
    gp.Update(0);
    assert(!gp.IsButtonDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0));
    assert(gp.IsButtonReleased(GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0));

    std::cout << "  -> Gamepad Processing PASSED." << std::endl;
}

void TestInputBuffering() {
    std::cout << "[TEST] 4. Input Buffering & Expiration..." << std::endl;

    InputBuffer buffer;
    buffer.SetBufferDuration(InputAction::PrimaryAttack, 0.120f); // 120 ms
    assert(std::abs(buffer.GetBufferDuration(InputAction::PrimaryAttack) - 0.120f) < 0.001f);

    // Initially unbuffered
    assert(!buffer.HasBufferedAction(InputAction::PrimaryAttack));

    // Buffer the attack action
    buffer.BufferAction(InputAction::PrimaryAttack);
    assert(buffer.HasBufferedAction(InputAction::PrimaryAttack));

    // Advance 50 ms (still active)
    buffer.Update(0.050f);
    assert(buffer.HasBufferedAction(InputAction::PrimaryAttack));

    // Consume the attack action
    bool consumed = buffer.ConsumeAction(InputAction::PrimaryAttack);
    assert(consumed);
    (void)consumed;

    // Consuming a second time immediately must fail (no duplicate consumption)
    assert(!buffer.ConsumeAction(InputAction::PrimaryAttack));
    assert(!buffer.HasBufferedAction(InputAction::PrimaryAttack));

    // Buffer again and test expiration
    buffer.BufferAction(InputAction::PrimaryAttack);
    assert(buffer.HasBufferedAction(InputAction::PrimaryAttack));

    // Advance 150 ms (> 120 ms buffer timeout)
    buffer.Update(0.150f);
    assert(!buffer.HasBufferedAction(InputAction::PrimaryAttack));
    assert(!buffer.ConsumeAction(InputAction::PrimaryAttack));

    std::cout << "  -> Input Buffering PASSED." << std::endl;
}

void TestInputContexts() {
    std::cout << "[TEST] 5. Input Contexts & Modal Exclusivity..." << std::endl;

    InputContextManager ctxMgr;
    assert(ctxMgr.GetActiveContextType() == InputContextType::Gameplay);

    // In Gameplay context: movement and combat are allowed
    assert(ctxMgr.IsActionAllowed(InputAction::MoveUp));
    assert(ctxMgr.IsActionAllowed(InputAction::PrimaryAttack));
    assert(ctxMgr.IsActionAllowed(InputAction::OpenInventory));

    // Push Menu context (modal)
    ctxMgr.PushContext(InputContextType::Menu);
    assert(ctxMgr.GetActiveContextType() == InputContextType::Menu);
    assert(ctxMgr.GetStackDepth() == 2);

    // In Menu context: UI navigation is allowed, but movement & combat are strictly blocked!
    assert(!ctxMgr.IsActionAllowed(InputAction::MoveUp));
    assert(!ctxMgr.IsActionAllowed(InputAction::PrimaryAttack));
    assert(ctxMgr.IsActionAllowed(InputAction::UIConfirm));
    assert(ctxMgr.IsActionAllowed(InputAction::UICancel));

    // Push Inventory context
    ctxMgr.PushContext(InputContextType::Inventory);
    assert(ctxMgr.GetActiveContextType() == InputContextType::Inventory);
    assert(!ctxMgr.IsActionAllowed(InputAction::MoveUp));
    assert(!ctxMgr.IsActionAllowed(InputAction::PrimaryAttack));
    assert(ctxMgr.IsActionAllowed(InputAction::OpenInventory));

    // Pop back to Menu
    ctxMgr.PopContext();
    assert(ctxMgr.GetActiveContextType() == InputContextType::Menu);

    // Pop back to Gameplay
    ctxMgr.PopContext();
    assert(ctxMgr.GetActiveContextType() == InputContextType::Gameplay);
    assert(ctxMgr.IsActionAllowed(InputAction::MoveUp));
    assert(ctxMgr.IsActionAllowed(InputAction::PrimaryAttack));

    std::cout << "  -> Input Contexts PASSED." << std::endl;
}

void TestCoordinateConversion() {
    std::cout << "[TEST] 6. Mouse Screen-to-Virtual Coordinate Conversion..." << std::endl;

    // Viewport with 640x360 virtual canvas scaled to 1280x720 window (2x uniform scale, no letterbox)
    graphics::Viewport vp(640, 360, 1280, 720);

    Vector2 screenCenter{ 640.0f, 360.0f };
    Vector2 virtualCenter = vp.ScreenToVirtual(screenCenter);
    assert(std::abs(virtualCenter.x - 320.0f) < 0.1f);
    assert(std::abs(virtualCenter.y - 180.0f) < 0.1f);

    // Viewport with letterboxing (1920x1080 window for 16:9 640x360 canvas scaled 3x)
    graphics::Viewport vpWidescreen(640, 360, 1920, 1080);
    Vector2 virtualWidescreen = vpWidescreen.ScreenToVirtual(Vector2{ 960.0f, 540.0f });
    assert(std::abs(virtualWidescreen.x - 320.0f) < 0.1f);
    assert(std::abs(virtualWidescreen.y - 180.0f) < 0.1f);

    std::cout << "  -> Coordinate Conversion PASSED." << std::endl;
}

void TestRebindingAndPersistence() {
    std::cout << "[TEST] 7. Rebinding, Conflict Detection & JSON Persistence..." << std::endl;

    InputSettings settings;
    settings.ResetToDefaults();

    // Default MoveUp is KEY_W
    assert(settings.GetPrimaryBinding(InputAction::MoveUp).code == KEY_W);

    // Test conflict detection: binding KEY_W to MoveDown should detect conflict with MoveUp
    InputBinding conflictBinding{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, KEY_W };
    InputAction conf = settings.FindConflict(conflictBinding, InputAction::MoveDown);
    assert(conf == InputAction::MoveUp);
    (void)conf;

    // Test rebinding MoveUp to KEY_E with conflict resolution
    settings.StartRebinding(InputAction::MoveUp, DeviceType::Keyboard, false);
    assert(settings.IsRebinding());

    InputBinding newBinding{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, KEY_E };
    bool applied = settings.ApplyRebinding(newBinding, true);
    assert(applied);
    (void)applied;
    assert(!settings.IsRebinding());
    assert(settings.GetPrimaryBinding(InputAction::MoveUp).code == KEY_E);

    // Test JSON persistence roundtrip
    const std::string testPath = "build/test_input_bindings.json";
    bool saved = settings.SaveToFile(testPath);
    assert(saved);
    (void)saved;
    assert(std::filesystem::exists(testPath));

    InputSettings loadedSettings;
    bool loaded = loadedSettings.LoadFromFile(testPath);
    assert(loaded);
    (void)loaded;
    assert(loadedSettings.GetPrimaryBinding(InputAction::MoveUp).code == KEY_E);

    // Clean up test file
    std::filesystem::remove(testPath);

    std::cout << "  -> Rebinding & Persistence PASSED." << std::endl;
}

void TestInputManagerIntegration() {
    std::cout << "[TEST] 8. InputManager End-to-End Simulation & Injection..." << std::endl;

    InputManager inputMgr;
    inputMgr.SetSimulated(true);

    // Inject MoveRight action
    inputMgr.InjectAction(InputAction::MoveRight, true, true, false, 1.0f);
    Vector2 moveVec = inputMgr.GetMovementVector(true);
    assert(moveVec.x > 0.99f && moveVec.y == 0.0f);

    // Inject PrimaryAttack
    inputMgr.InjectAction(InputAction::PrimaryAttack, true, true, false, 1.0f);
    assert(inputMgr.WasActionPressed(InputAction::PrimaryAttack));
    assert(inputMgr.HasBufferedAction(InputAction::PrimaryAttack));

    // Consume the attack
    bool consumed = inputMgr.ConsumeAction(InputAction::PrimaryAttack);
    assert(consumed);
    (void)consumed;
    assert(!inputMgr.WasActionPressed(InputAction::PrimaryAttack));

    // Context switch to Menu: movement must immediately be blocked
    inputMgr.PushContext(InputContextType::Menu);
    Vector2 menuMove = inputMgr.GetMovementVector(true);
    assert(menuMove.x == 0.0f && menuMove.y == 0.0f);

    std::cout << "  -> InputManager Integration PASSED." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 21: ADVANCED INPUT SYSTEM TEST SUITE" << std::endl;
    std::cout << "============================================================" << std::endl;

    TestInputActions();
    TestInputBindings();
    TestGamepadProcessing();
    TestInputBuffering();
    TestInputContexts();
    TestCoordinateConversion();
    TestRebindingAndPersistence();
    TestInputManagerIntegration();

    std::cout << "============================================================" << std::endl;
    std::cout << "ALL 8 INPUT SYSTEM TEST SUITES PASSED SUCCESSFULLY (100%)" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
