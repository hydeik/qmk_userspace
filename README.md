# QMK Userspace (hydeik's personal keymap)

This is my custom mechanical keyboard firmware built upon QMK Firmware. 
This project makes use of [QMK userspace](https://github.com/qmk/qmk_userspace) feature, enabling code reuse and centralized feature definitions across multiple keymaps or keyboards.


## Howto configure your build targets

1. Run the normal `qmk setup` procedure if you haven't already done so -- see [QMK Docs](https://docs.qmk.fm/#/newbs) for details.
1. Fork this repository
1. Clone your fork to your local machine
1. Enable userspace in QMK config using `qmk config user.overlay_dir="$(realpath qmk_userspace)"`
1. Add a new keymap for your board using `qmk new-keymap`
    * This will create a new keymap in the `keyboards` directory, in the same location that would normally be used in the main QMK repository. For example, if you wanted to add a keymap for the Planck, it will be created in `keyboards/planck/keymaps/<your keymap name>`
    * You can also create a new keymap using `qmk new-keymap -kb <your_keyboard> -km <your_keymap>`
    * Alternatively, add your keymap manually by placing it in the location specified above.
    * `layouts/<layout name>/<your keymap name>/keymap.*` is also supported if you prefer the layout system
1. Add your keymap(s) to the build by running `qmk userspace-add -kb <your_keyboard> -km <your_keymap>`
    * This will automatically update your `qmk.json` file
    * Corresponding `qmk userspace-remove -kb <your_keyboard> -km <your_keymap>` will delete it
    * Listing the build targets can be done with with `qmk userspace-list`
1. Commit your changes

## Howto build with GitHub

No local QMK installation is needed: every push is built by GitHub Actions (`.github/workflows/build_binaries.yaml`).

1. In the GitHub Actions tab, enable workflows
1. Push your changes to GitHub (any branch), or start the workflow manually from the Actions tab
1. Wait for the actions run to complete
1. Download the firmware:
    * From the `latest` release on the Releases tab. It is replaced only by builds of `main` in which every build target succeeded, so a failed build never removes previously published firmware.
    * From the `firmware-qmk` and `firmware-vial` artifacts of the actions run, for other branches.

Each build target in `qmk.json` is built against exactly one firmware source, chosen by its keymap name:

| Keymap name | Firmware source |
| --- | --- |
| ends with `_vial` | [`vial-kb/vial-qmk`](https://github.com/vial-kb/vial-qmk), branch `vial` |
| anything else | [`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware), branch `master` |

Both sources are used at their latest revision at build time; they are not pinned.

## Howto build locally

1. Run the normal `qmk setup` procedure if you haven't already done so -- see [QMK Docs](https://docs.qmk.fm/#/newbs) for details.
1. Fork this repository
1. Clone your fork to your local machine
1. `cd` into this repository's clone directory
1. Set global userspace path: `qmk config user.overlay_dir="$(realpath .)"` -- you MUST be located in the cloned userspace location for this to work correctly
    * This will be automatically detected if you've `cd`ed into your userspace repository, but the above makes your userspace available regardless of your shell location.
1. Compile normally: `qmk compile -kb your_keyboard -km your_keymap` or `make your_keyboard:your_keymap`

Alternatively, if you configured your build targets above, you can use `qmk userspace-compile` to build all of your userspace targets at once.

## Extra info

To build against a different repository or branch, edit the `matrix` of the `build` job in `.github/workflows/build_binaries.yaml`:
```
          - name: qmk
            repo: qmk/qmk_firmware
            ref: master
            vial: false
```

##License
This firmware is licensed under the GNU General Public License v2.0.

