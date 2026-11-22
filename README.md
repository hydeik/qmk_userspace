# QMK Userspace (hydeik's personal keymaps)

My keyboard firmware configuration, as a [QMK userspace](https://github.com/qmk/qmk_userspace): the keymaps, their shared behaviour, and the build setup for the keyboards I type on. Every keymap has the same 34 keys.

The terms used here (keymap family, keymap, layout, firmware source, build target) are defined in [`GLOSSARY.md`](GLOSSARY.md). Why the repository is organised this way is recorded in [`docs/adr/`](docs/adr/).

## Keymap family

A keymap family is a complete typing design that can be applied to more than one keyboard. There is one:

| Keymap family | Design | Layers |
| --- | --- | --- |
| `hydeik` | No home-row mods; custom one-shot mods and a smart mod layer, and layer switching with holding keys and trigger keys. | BASE, SYM, NUM, NAV, MOU, FUN, MOD |

The layers, their keys, how each layer is reached and how the one-shot mods and the smart mod layer behave are described in [`users/hydeik/README.md`](users/hydeik/README.md). MOU is a layer of mouse keys, so Mouse Keys are switched on for every keymap. That there is one keymap family in one directory, built with and without Vial, is decided in [ADR 0004](docs/adr/0004-one-keymap-family-built-with-and-without-vial.md).

A keymap is the keymap family applied to one keyboard, either with or without Vial. The keymap with Vial is named `hydeik_vial` and is built against [`vial-kb/vial-qmk`](https://github.com/vial-kb/vial-qmk), branch `vial`; it adds Vial's GUI for remapping and tap-hold settings. The keymap without Vial is named `hydeik` and is built against upstream [`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware), branch `master`.

### With and without Vial

Both keymaps have the same layers, key for key, and the same one-shot mods, smart mod layer and layer switching. They differ only in what Vial brings:

| | `hydeik` (without Vial) | `hydeik_vial` (with Vial) |
| --- | --- | --- |
| Firmware source | `qmk` | `vial-qmk` |
| Remapping | None. The keyboard types with the firmware keymap. | In the Vial GUI, which edits the stored keymap. On its first boot, every newly built firmware resets the stored keymap to the firmware keymap. |
| Tap-hold settings | Fixed at build time in `users/hydeik/config.h`: a tapping term of 200 ms and Hold On Other Key Press. | Vial settings, changed in the Vial GUI. After a settings reset they are the same as without Vial: `firmware_default` switches Hold On Other Key Press on. The `HOLD_ON_OTHER_KEY_PRESS` of `config.h` is ignored. |
| Combos and tap dances | Off. | On, for use from the Vial GUI. The firmware defines none. |

Apart from Vial, the keymap `hydeik_vial` of the crkbd has RGB lighting and an encoder map, and the keymaps `hydeik` have neither.

## Repository structure

| Path | Contents |
| --- | --- |
| `users/hydeik/` | The `hydeik` keymap family, for every keymap of the family. |
| `users/hydeik/README.md` | The description of the keymap family: its layers and its behaviour. |
| `users/hydeik/rules.mk`, `config.h` | Its build settings and its settings. |
| `users/hydeik/layout.h` | The layer numbers, the keycodes and the layers. |
| `users/hydeik/hydeik.c`, `hydeik.h` | The glue between QMK and the two state machines, with the QMK hooks. |
| `users/hydeik/typing_rules.c` | The Caps Word and Repeat Key rules. |
| `users/hydeik/vial_hooks.c` | The QMK hooks that exist only with Vial. |
| `users/hydeik/features/` | `smart_mod`, the one-shot mods and the smart mod layer; `target_layer`, the layer switching; `key_typed`, what counts as a key typed; `firmware_default`, the firmware defaults of the Vial settings. |
| `users/hydeik/keyboards/` | One layout adapter per keyboard, which places the 34 keys of a layer in the layout of that keyboard. |
| `users/hydeik/tests/` | The [unit tests](#unit-tests). |
| `keyboards/<keyboard>/keymaps/<keymap>/` | One keymap: the keymap family applied to one keyboard. |
| `qmk.json` | The build targets. |
| `firmware_sources.json` | The firmware sources, each pinned to a commit and listing the keymaps built against it. |
| `Makefile` | The [local builds](#building-locally-with-make) and the [unit tests](#unit-tests). |
| `.github/workflows/build_binaries.yaml` | The [CI workflow](#building-with-github-actions), with its script `.github/scripts/firmware_sources.py`. |
| `GLOSSARY.md`, `docs/adr/` | The terms and the recorded decisions. |

What exists only with Vial is chosen in `users/hydeik/rules.mk`: it adds `vial_hooks.c` and `features/firmware_default.c`, and switches combos and tap dances on, only for a keymap with Vial. No C source branches on Vial.

## Build targets

A build target is one keyboard paired with one keymap, and produces one firmware binary. `qmk.json` is the complete list, and today it holds three:

| Keyboard | Keymap | Firmware source | Firmware binary |
| --- | --- | --- | --- |
| `crkbd/rev4_1/mini` | `hydeik` | `qmk` | `crkbd_rev4_1_mini_hydeik.uf2` |
| `crkbd/rev4_1/mini` | `hydeik_vial` | `vial-qmk` | `crkbd_rev4_1_mini_hydeik_vial.uf2` |
| `mothwing` | `hydeik` | `qmk` | `mothwing_hydeik.hex` |

The keymap name of a build target says whether it is built with Vial, and selects its [firmware source](#firmware-sources-and-pins). Every keymap directory under `keyboards/` must be a build target.

To add a build target, create `keyboards/<keyboard>/keymaps/<keymap>/` and add `["<keyboard>", "<keymap>"]` to `build_targets` in `qmk.json`. A keymap directory that is not named `hydeik` sets `USER_NAME := hydeik` in its `rules.mk`, so that it uses `users/hydeik/`. If the keyboard is new, also add its layout adapter to `users/hydeik/keyboards/`.

## Building with GitHub Actions

This is the official build route. Every push is built by `.github/workflows/build_binaries.yaml`, and no local QMK installation is needed.

1. Push your changes to GitHub (any branch), or start the workflow manually from the Actions tab.
1. Wait for the actions run to complete.
1. Download the firmware:
    * From the `latest` release on the Releases tab. It is replaced only by builds of `main` in which every build target succeeded and the [unit tests](#unit-tests) passed, so a failed build or a failed test never removes previously published firmware.
    * From the `firmware-<firmware source>` artifacts of the actions run, for other branches. Today there are two, `firmware-qmk` and `firmware-vial-qmk`.

The workflow runs one `Build` job per firmware source that has build targets, and one `Test` job per firmware source that unit tests run on: `Build (qmk)`, `Build (vial-qmk)`, `Test (qmk)` and `Test (vial-qmk)`. It fails if the keymap of any build target in `qmk.json` is listed by no firmware source in `firmware_sources.json`, or by more than one.

A further job, `Keymap image`, runs the unit tests of the script that writes the keymap image and checks that the committed `users/hydeik/image/keymap.svg` is the one `make keymap-image` writes. If the image is stale, the job fails and says to run `make keymap-image` and commit the result; it does not regenerate the image. It does not gate the `latest` release: a stale keymap image turns the workflow red but never holds back a firmware release.

## Firmware sources and pins

`firmware_sources.json` declares the firmware sources by name: `qmk` is upstream QMK and `vial-qmk` is vial-qmk. Each entry has four fields:

* `repo`: the firmware repository on GitHub.
* `branch`: the branch of `repo` that the pin follows. The build does not read it; it records where a newer `ref` is taken from.
* `ref`: the pin, a full commit SHA. Every build checks out exactly this commit, so the same commit of this repository always builds against the same firmware source.
* `keymaps`: the names of the keymaps that are built against this firmware source.

The keymap name of a build target, matched exactly, selects its firmware source: the one whose `keymaps` list holds it. Today `qmk` lists `hydeik` and `vial-qmk` lists `hydeik_vial`. A keymap that no firmware source lists, or that more than one lists, is an error. Nothing is inferred from the form of the name, and there is no default firmware source, so a keymap with a new name must be added to one `keymaps` list before it can be built.

A firmware source only changes when its pin is moved on purpose. To bump a pin:

1. Create a branch.
1. In `firmware_sources.json`, set the firmware source's `ref` to a newer commit of its `branch`. The commit at the head of the branch is printed by, for example, `git ls-remote https://github.com/qmk/qmk_firmware refs/heads/master`.
1. Push the branch and wait for the actions run.
1. Merge to `main` once CI passes. The `latest` release is replaced only by a successful build of `main`, so a bad bump never reaches the published firmware.

To take a firmware source from a different repository or branch, edit `repo`, `branch` and `ref` of its entry in the same way. To build a keymap against the other firmware source, move its name to the other `keymaps` list.

## Building locally with make

Local builds are a secondary route, for work that needs many flash-and-try rounds. They go through `make` only, which builds against the same pinned firmware sources as CI.

The toolchain is not managed by this repository. Install the QMK CLI and compilers the [official QMK way](https://docs.qmk.fm/newbs_getting_started) for your OS; on macOS, `brew install qmk/qmk/qmk`. `make`, `git` and `python3` are also needed.

1. Fetch the firmware sources:

    ```
    make firmware
    ```

    This clones every firmware source into `.firmware/<firmware source>/`, that is `.firmware/qmk/` and `.firmware/vial-qmk/`, and checks out its pinned commit with submodules. Run it again after a pin is bumped. `.firmware/` is git-ignored and takes two firmware checkouts of disk space; delete the directory to remove it.

1. Build a build target, or build and flash it:

    ```
    make crkbd/rev4_1/mini:hydeik_vial
    make crkbd/rev4_1/mini:hydeik_vial:flash
    make mothwing:hydeik
    ```

    The keymap name is looked up in the `keymaps` lists of `firmware_sources.json`, and the target is handed to the own `make` of the firmware source that lists it, with `QMK_USERSPACE` set to this repository. The build stops with a message that names the keymap if no firmware source lists it, or more than one does, and with a message to run `make firmware` if the checkout or one of its submodules is missing or not at its pinned commit.

Building with `qmk compile` or `qmk flash` is not supported: they build against the single firmware checkout the QMK CLI is configured with, which cannot serve two firmware sources, and nothing checks the pin.

## Unit tests

Unit tests drive key events through the unit-test framework of a firmware source and check the keyboard reports sent to the host. They are in `users/hydeik/tests/`, one directory per test:

* `target_layer`: the `target_layer` state machine (`users/hydeik/features/target_layer.c`), which switches to a target layer.
* `smart_mod`: the `smart_mod` state machine (`users/hydeik/features/smart_mod.c`), which holds the one-shot mods and the smart mod layer.
* `target_layer_smart_mod`: `smart_mod` together with `target_layer`, wired as in the firmware and on the layers of the family.
* `hold_on_other_key_press`: the same two, wired the same way, with Hold On Other Key Press: the gestures that need the setting, inside the tapping term.
* `firmware_default`: the firmware defaults of the Vial settings.

All of them run on the unit-test frameworks of both firmware sources, `qmk` and `vial-qmk`, except `firmware_default`, which runs on `vial-qmk` only.

Run them all with:

```
make test
```

This needs the firmware checkouts from `make firmware` and a native C/C++ compiler (`gcc`; on macOS the Xcode command line tools), but neither the QMK CLI nor a cross-compiler for the keyboard. The path of this repository must not contain spaces, which the test makefile of the firmware source cannot handle. Like a build, it stops with a message to run `make firmware` if a firmware checkout it needs is missing or not at its pinned commit, before anything is built. It fails if any test fails, or if a test binary contains no test.

Each directory `users/hydeik/tests/<test>/` with a `test.mk` is one test binary, and every `*.cpp` file in it is compiled into that binary. The test directory is handed to the test makefile of the firmware source; nothing is placed in the firmware checkout except its own `.build/` output. The tests are built without Vial and with QMK's stock tap-hold behaviour, and the `config.h` of a test mirrors the settings it depends on by hand. There are two exceptions: `hold_on_other_key_press` mirrors the `HOLD_ON_OTHER_KEY_PRESS` of `users/hydeik/config.h` as well, and `firmware_default` also compiles the module of vial-qmk that holds the Vial settings.

Every test names the firmware sources it runs on with one line in its `test.mk`, by their names in `firmware_sources.json`:

```
TEST_FIRMWARE_SOURCES = qmk vial-qmk
```

`make test` reads that line as text, so it must be a single assignment of literal names. The line is required: there is no default, and `make test` fails with a message that names the `test.mk` that lacks it.

`make test FIRMWARE_SOURCE=qmk` or `make test FIRMWARE_SOURCE=vial-qmk` runs only the tests that run on that firmware source, and needs only that firmware checkout. CI runs the tests this way, once per firmware source, in the `Test` jobs of the workflow.

## License

This firmware is licensed under the GNU General Public License v2.0.
