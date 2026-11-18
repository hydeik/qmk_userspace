/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hydeik.h"
#include "users/hydeik_common/keyboards/crkbd_rev4_1_mini.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_HYDEIK_34(BASE_LAYER),
    [_SYM] = LAYOUT_HYDEIK_34(SYM_LAYER),
    [_NUM] = LAYOUT_HYDEIK_34(NUM_LAYER),
    [_NAV] = LAYOUT_HYDEIK_34(NAV_LAYER),
    [_MOU] = LAYOUT_HYDEIK_34(MOUSE_LAYER),
    [_FUN] = LAYOUT_HYDEIK_34(FUN_LAYER),
};
