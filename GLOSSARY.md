# QMK Userspace

Personal keyboard firmware configuration: the keymaps, shared behaviour, and build setup for the keyboards its owner types on.

## Language

**Keymap family**:
A complete typing design (its layers, modifier scheme, and custom key behaviour) that is applied to several keyboards. There are two: `hydeik` and `hydeik_vial`.
_Avoid_: Layout, keyboard layout, variant, flavour

**Common core**:
The definitions and behaviour that are deliberately kept identical in both keymap families, held in one place that each family draws on.
_Avoid_: Shared code, common, shared, base

**Layout**:
The physical arrangement of keys on one keyboard, as exposed by that keyboard's `LAYOUT` macro.
_Avoid_: Using "layout" for a keymap family

**Firmware source**:
The firmware repository and branch a build target is compiled against, either upstream QMK or vial-qmk.
_Avoid_: Firmware, backend, fork

**Build target**:
One keyboard paired with one keymap, producing one firmware binary.
