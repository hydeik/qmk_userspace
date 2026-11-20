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
_Avoid_: OSM, sticky key, COS key

**Pending one-shot mod**:
A one-shot mod that has been tapped and not yet consumed, cancelled, or timed out. The host does not see it until the key press that consumes it.
_Avoid_: Queued mod, armed mod
