# QMK Userspace

Personal keyboard firmware configuration: the keymaps, shared behaviour, and build setup for the keyboards its owner types on.

## Language

**Keymap family**:
A complete typing design (its layers, modifier scheme, and custom key behaviour) that can be applied to more than one keyboard. There are two: `hydeik` and `hydeik_vial`.
_Avoid_: Layout, keyboard layout, variant, flavour

**Common core**:
The definitions and behaviour that do not depend on the keymap family, held in one place that each family draws on. What both families use is deliberately kept identical.
_Avoid_: Shared code, common, shared, base

**Layout**:
The physical arrangement of keys on one keyboard, as exposed by that keyboard's `LAYOUT` macro.
_Avoid_: Using "layout" for a keymap family

**Firmware source**:
The firmware repository and branch a build target is compiled against, either upstream QMK or vial-qmk. Each keymap family is built against exactly one.
_Avoid_: Firmware, backend, fork

**Build target**:
One keyboard paired with one keymap, producing one firmware binary.

**One-shot mod**:
A modifier key of the `hydeik_vial` keymap family that acts as an ordinary modifier while held and, when tapped, applies to the next key press only.
_Avoid_: OSM, sticky key, COS key, QMK's built-in one-shot mods (a different mechanism)

**Pending one-shot mod**:
A one-shot mod that has been tapped and not yet consumed, cancelled, or timed out. The host does not see it until the key press that consumes it.
_Avoid_: Queued mod, armed mod

**Smart mod layer**:
The layer of one-shot mods reached by holding a smart mod layer key. It switches itself off as soon as the chosen one-shot mods are released, so the keys beneath it can be typed with them.
_Avoid_: MOD layer (as a concept name), one-shot mod layer

**Smart mod layer key**:
A thumb key that reaches the smart mod layer while held and keeps the chosen one-shot mods latched until it is released.
_Avoid_: Layer key, mod key

**Latched one-shot mod**:
A one-shot mod chosen on the smart mod layer that stays active, visible to the host, for as long as a smart mod layer key is held.
_Avoid_: Used mod, locked mod, sticky mod

**Holding key**:
A thumb key that keeps a layer on while it is held and, for that time, makes its trigger keys work.
_Avoid_: Layer key, hold key, suppressor

**Holding layer**:
The layer that a holding key keeps on while it is held.
_Avoid_: Secondary layer

**Trigger key**:
A key that, tapped while one of its holding keys is held, switches to its target layer. Without a held holding key it does nothing.
_Avoid_: Layer switch key, `TO` key

**Target layer**:
The layer that a trigger key switches to. It stays on for as long as the holding key is held.
_Avoid_: Tertiary layer

**Pending target layer**:
A target layer left on after its holding key was released quickly, with no key typed. It applies to the next key press only.
_Avoid_: One-shot layer, QMK's built-in one-shot layers (a different mechanism)

**Locked target layer**:
A target layer that stays on after its holding key was released quickly, with no key typed, until it is left on purpose. Which target layers lock and which become pending is chosen per trigger key.
_Avoid_: Sticky layer, toggled layer
