# QMK Userspace (hydeik's personal keymaps)

My keyboard firmware configuration, as a [QMK userspace](https://github.com/qmk/qmk_userspace): the keymaps, their shared behaviour, and the build setup for the keyboards I type on. Every keymap uses 34 keys.

The terms used here (keymap family, common core, layout, firmware source, build target) are defined in [`GLOSSARY.md`](GLOSSARY.md). Why the repository is organised this way is recorded in [`docs/adr/`](docs/adr/).

## Keymap families

A keymap family is a complete typing design that can be applied to more than one keyboard. There are two, and each is built against exactly one firmware source:

| Keymap family | Firmware source | Design | Layers |
| --- | --- | --- | --- |
| `hydeik` | [`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware), branch `master` | Home-row mods with tuned tap-hold behaviour. See [`users/hydeik/README.md`](users/hydeik/README.md). | BASE, SYM, NUM, NAV, MOU, FUN |
| `hydeik_vial` | [`vial-kb/vial-qmk`](https://github.com/vial-kb/vial-qmk), branch `vial` | No home-row mods; custom one-shot mods on a smart mod layer, and Vial's GUI for remapping and tap-hold settings. | BASE, SYM, NUM, NAV, FUN, MOD |

## Repository structure

| Path | Contents |
| --- | --- |
| `users/hydeik/` | The `hydeik` keymap family: its layers, keycodes and behaviour. |
| `users/hydeik_vial/` | The `hydeik_vial` keymap family. Its [unit tests](#unit-tests) are in `tests/`. |
| `users/hydeik_common/` | The common core: everything that does not depend on the keymap family. Settings (`common_config.h`, `common.mk`), the Caps Word and Repeat Key rules (`common.c`), what counts as a key typed and the layer-switching state machine (`features/`), the finger keys of the SYM, NUM and FUN layers (`layers.h`), one layout adapter per keyboard (`keyboards/`), and its [unit tests](#unit-tests) (`tests/`). |
| `keyboards/<keyboard>/keymaps/<keymap family>/` | One keymap: a keymap family applied to one keyboard. |
| `qmk.json` | The build targets. |
| `firmware_sources.json` | The firmware source of each keymap family, pinned to a commit. |

Each keymap family includes the common core by its full path from the repository root. The common core never refers to a keymap family and never branches on the keymap family or the firmware source.

## Build targets

A build target is one keyboard paired with one keymap, and produces one firmware binary. `qmk.json` is the complete list:

| Keyboard | Keymap |
| --- | --- |
| `crkbd/rev4_1/mini` | `hydeik` |
| `crkbd/rev4_1/mini` | `hydeik_vial` |
| `mothwing` | `hydeik` |

A build target's keymap name is its keymap family. Every keymap directory under `keyboards/` must be a build target; not every keymap family exists on every keyboard.

To add a build target, create `keyboards/<keyboard>/keymaps/<keymap family>/` and add `["<keyboard>", "<keymap family>"]` to `build_targets` in `qmk.json`. If the keyboard is new, also add its layout adapter to `users/hydeik_common/keyboards/`.

## Building with GitHub Actions

This is the official build route. Every push is built by `.github/workflows/build_binaries.yaml`, and no local QMK installation is needed.

1. Push your changes to GitHub (any branch), or start the workflow manually from the Actions tab.
1. Wait for the actions run to complete.
1. Download the firmware:
    * From the `latest` release on the Releases tab. It is replaced only by builds of `main` in which every build target succeeded and the [unit tests](#unit-tests) passed, so a failed build or a failed test never removes previously published firmware.
    * From the `firmware-<keymap family>` artifacts of the actions run (`firmware-hydeik` and `firmware-hydeik_vial`), for other branches.

The workflow fails if any build target in `qmk.json` has a keymap name with no entry in `firmware_sources.json`.

## Firmware sources and pins

`firmware_sources.json` declares the firmware source of each keymap family. Each entry has three fields:

* `repo`: the firmware repository on GitHub.
* `branch`: the branch of `repo` that the pin follows. The build does not read it; it records where a newer `ref` is taken from.
* `ref`: the pin, a full commit SHA. Every build checks out exactly this commit, so the same commit of this repository always builds against the same firmware source.

A firmware source only changes when its pin is moved on purpose. To bump a pin:

1. Create a branch.
1. In `firmware_sources.json`, set the keymap family's `ref` to a newer commit of its `branch`. The commit at the head of the branch is printed by, for example, `git ls-remote https://github.com/qmk/qmk_firmware refs/heads/master`.
1. Push the branch and wait for the actions run.
1. Merge to `main` once CI passes. The `latest` release is replaced only by a successful build of `main`, so a bad bump never reaches the published firmware.

To build a keymap family against a different repository or branch, edit `repo`, `branch` and `ref` of its entry in the same way.

## Building locally with make

Local builds are a secondary route, for work that needs many flash-and-try rounds. They go through `make` only, which builds against the same pinned firmware sources as CI.

The toolchain is not managed by this repository. Install the QMK CLI and compilers the [official QMK way](https://docs.qmk.fm/newbs_getting_started) for your OS; on macOS, `brew install qmk/qmk/qmk`. `make`, `git` and `python3` are also needed.

1. Fetch the firmware sources:

    ```
    make firmware
    ```

    This clones the firmware source of every keymap family into `.firmware/<keymap family>/` and checks out its pinned commit with submodules. Run it again after a pin is bumped. `.firmware/` is git-ignored and takes two firmware checkouts of disk space; delete the directory to remove it.

1. Build a build target, or build and flash it:

    ```
    make crkbd/rev4_1/mini:hydeik
    make crkbd/rev4_1/mini:hydeik_vial:flash
    ```

    The keymap name selects the firmware checkout, and the target is handed to that firmware source's own `make` with `QMK_USERSPACE` set to this repository. The build stops with a message to run `make firmware` if the checkout or one of its submodules is missing or not at its pinned commit.

Building with `qmk compile` or `qmk flash` is not supported: they build against the single firmware checkout the QMK CLI is configured with, which cannot serve two firmware sources, and nothing checks the pin.

## Unit tests

Unit tests drive key events through the unit-test framework of a firmware source and check the keyboard reports sent to the host. There are two kinds:

* Tests of the common core, in `users/hydeik_common/tests/`: its `target_layer` state machine (`users/hydeik_common/features/target_layer.c`), which switches to a target layer. They run on the unit-test frameworks of both firmware sources, upstream QMK and vial-qmk.
* Tests of the `hydeik_vial` keymap family, in `users/hydeik_vial/tests/`: its `smart_mod` state machine (`users/hydeik_vial/features/smart_mod.c`), which holds its one-shot mods and its smart mod layer; `smart_mod` together with `target_layer`, wired as in the firmware and on the keymap of the family (`target_layer_smart_mod`); and the firmware defaults of its Vial settings (`firmware_default`). They run on vial-qmk only.

Run them all with:

```
make test
```

This needs the firmware checkouts from `make firmware` and a native C/C++ compiler (`gcc`; on macOS the Xcode command line tools), but neither the QMK CLI nor a cross-compiler for the keyboard. The path of this repository must not contain spaces, which the test makefile of the firmware source cannot handle. Like a build, it stops with a message to run `make firmware` if a firmware checkout it needs is missing or not at its pinned commit, before anything is built. It fails if any test fails, or if a test binary contains no test.

Each directory `users/<owner>/tests/<test>/` with a `test.mk` is one test binary, where `<owner>` is a keymap family or `hydeik_common`, and every `*.cpp` file in it is compiled into that binary. The test directory is handed to the test makefile of the firmware source; nothing is placed in the firmware checkout except its own `.build/` output. The tests are built without Vial, so with QMK's stock tap-hold behaviour, and the `config.h` of a test mirrors the settings it depends on by hand. The one exception is `firmware_default`, which also compiles the module of vial-qmk that holds the Vial settings.

A test names the firmware sources it runs on with one line in its `test.mk`, each firmware source by the keymap family that declares it in `firmware_sources.json`:

```
TEST_FIRMWARE_SOURCES = hydeik hydeik_vial
```

`make test` reads that line as text, so it must be a single assignment of literal names. A test of a keymap family may leave it out, and then runs on the firmware source of that family. A test of the common core must have it.

`make test FIRMWARE_SOURCE=hydeik` runs only the tests that run on the firmware source of that keymap family, and needs only that firmware checkout. CI runs the tests this way, once per firmware source, in the `Test` jobs of the workflow.

## License

This firmware is licensed under the GNU General Public License v2.0.
