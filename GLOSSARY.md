# QMK Userspace

Personal keyboard firmware configuration: the keymaps, shared behaviour, and build setup for the keyboards its owner types on.

## Language

**Keymap family**:
A complete typing design (its layers, modifier scheme, and custom key behaviour) that can be applied to more than one keyboard. There is one: `hydeik`.
_Avoid_: Layout, keyboard layout, variant, flavour

**Keymap**:
A keymap family applied to one keyboard, either with or without Vial. `hydeik` is the keymap without Vial, `hydeik_vial` the keymap with Vial.

**Layout**:
The physical arrangement of keys on one keyboard, as exposed by that keyboard's `LAYOUT` macro.
_Avoid_: Using "layout" for a keymap family

**Firmware source**:
The firmware repository and branch a build target is compiled against, either upstream QMK or vial-qmk. Each build target is built against exactly one, chosen by its keymap.
_Avoid_: Firmware, backend, fork

**Build target**:
One keyboard paired with one keymap, producing one firmware binary.

**Firmware keymap**:
The keymap that is built into a firmware binary. It is what a keyboard types with when nothing else is stored, and what a reset goes back to.
_Avoid_: Default keymap, compiled keymap

**Stored keymap**:
The keymap that Vial keeps on the keyboard and lets the owner edit while the keyboard runs. It starts as a copy of the firmware keymap and goes back to it on the first boot of every newly built firmware. Only build targets with Vial have one.
_Avoid_: Dynamic keymap, EEPROM keymap, Vial keymap

**Keymap image**:
The picture of the firmware keymap of a keymap family: all its layers, drawn from the sources. It is not a picture of the keymap of one keyboard, nor of a stored keymap.
_Avoid_: Rendered image, diagram (a diagram is the drawing of a layer in the comments of `layout.h`)

**One-shot mod**:
A modifier key that acts as an ordinary modifier while held and, when tapped, applies to the next key press only.
_Avoid_: OSM, sticky key, COS key, one-shot modifier

**Pending one-shot mod**:
A one-shot mod that has been tapped and not yet consumed, cancelled, or timed out. The host does not see it until the key press that consumes it.
_Avoid_: Queued mod, armed mod

**One Shot Term**:
The short time within which a one-shot gesture has to be finished. A one-shot mod key released within it was tapped, and a holding key released within it, counted from the press of the trigger key, was released quickly.
_Avoid_: Tapping term (QMK's tap-hold setting), timeout (how long a pending one-shot mod or a pending target layer lasts)

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

**Quick release**:
The release of the last held holding key within the One Shot Term after the press of the trigger key, with no key typed in between. It can leave the target layer pending or locked instead of switching it off.
_Avoid_: Tap (of a holding key), fast release

**Pending target layer**:
A target layer left on after its holding key was released quickly, with no key typed. It applies to the next key press only.
_Avoid_: One-shot layer, QMK's built-in one-shot layers (a different mechanism)

**Locked target layer**:
A target layer that stays on after its holding key was released quickly, with no key typed, until it is left on purpose. Which target layers lock and which become pending is chosen per trigger key.
_Avoid_: Sticky layer, toggled layer

**Indicator**:
A light on a key that shows a state of the one-shot mods or of the layer switching. It is drawn over the ordinary lighting effect of the keyboard.
_Avoid_: LED indicator (QMK's word for the lock lights of the host, such as Caps Lock), status light, RGB indicator

**Indicator state**:
What the indicator at one key position shows: a pending one-shot mod, a latched one-shot mod, a trigger key that works now, a target layer that is on, a pending target layer, or a locked target layer. A key position shows at most one.
_Avoid_: Indicator colour (the colour is how a state looks, not the state)
