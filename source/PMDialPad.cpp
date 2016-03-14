/*
 * Copyright (c) 2016, Martin Gysel, me@bearsh.org
 * SPDX-License-Identifier: GPL-3.0+
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pmdailpad/PMDialpad.h"

#define AIN_VALID_THRESHOLD   3
#define DEBOUNCE_TIME_MS      20
#define DEBOUNCE_TIME_US      (DEBOUNCE_TIME_MS * 1000)
#define RECHECK_TIME_US       (100 * 1000)
#define BUTTON_PRESS_LONG_US  (500 * 1000)

#define ARRAY_SIZE(x)         (sizeof(x)/sizeof(x[0]))


struct range {
	PMDialPad::Key key;
	uint16_t low;
	uint16_t high;
};


#define ADC_KEY_S  5    // 0
#define ADC_KEY_1  30
#define ADC_KEY_4  55
#define ADC_KEY_7  76
#define ADC_KEY_0  93
#define ADC_KEY_2  109
#define ADC_KEY_5  122
#define ADC_KEY_8  135
#define ADC_KEY_R  145
#define ADC_KEY_3  154
#define ADC_KEY_6  163
#define ADC_KEY_9  171

#define ADC_LOW_HIGH(x)   x-5, x+5

static const struct range ranges[PMDialPad::KEY_LIST_LEN] = {
	{PMDialPad::KEY_S, ADC_LOW_HIGH(ADC_KEY_S)},
	{PMDialPad::KEY_1, ADC_LOW_HIGH(ADC_KEY_1)},
	{PMDialPad::KEY_4, ADC_LOW_HIGH(ADC_KEY_4)},
	{PMDialPad::KEY_7, ADC_LOW_HIGH(ADC_KEY_7)},
	{PMDialPad::KEY_0, ADC_LOW_HIGH(ADC_KEY_0)},
	{PMDialPad::KEY_2, ADC_LOW_HIGH(ADC_KEY_2)},
	{PMDialPad::KEY_5, ADC_LOW_HIGH(ADC_KEY_5)},
	{PMDialPad::KEY_8, ADC_LOW_HIGH(ADC_KEY_8)},
	{PMDialPad::KEY_R, ADC_LOW_HIGH(ADC_KEY_R)},
	{PMDialPad::KEY_3, ADC_LOW_HIGH(ADC_KEY_3)},
	{PMDialPad::KEY_6, ADC_LOW_HIGH(ADC_KEY_6)},
	{PMDialPad::KEY_9, ADC_LOW_HIGH(ADC_KEY_9)},
	{PMDialPad::KEY_NONE, ADC_KEY_9+10, 255},
};


static PMDialPad::Key keymap(uint16_t val) {
	int i = ARRAY_SIZE(ranges) / 2;

	while ((i >= 0) && (i < (int)ARRAY_SIZE(ranges))) {
		if (val < ranges[i].low) {
			i--;
			if (val > ranges[i].high) {
				return PMDialPad::KEY_INVALID;
			}
		} else if (val > ranges[i].high) {
			i++;
			if (val < ranges[i].low) {
				return PMDialPad::KEY_INVALID;
			}
		} else {  // (val >= ranges[i].low) && (val <= ranges[i].high)
			return ranges[i].key;
		}
	}
	return PMDialPad::KEY_INVALID;
}

PMDialPad::PMDialPad(PinName taster, PinName scale) :
		button(KEY_NONE),
		anaIn(taster),
		intIn(taster),
		scaleOut(scale),
		to(),
		status(IDLE),
		convert_run(0)
{
	anaIn.config(ADC_CONFIG_RES_10bit, ADC_CONFIG_INPSEL_AnalogInputTwoThirdsPrescaling,
			ADC_CONFIG_REFSEL_SupplyOneHalfPrescaling, ADC_CONFIG_EXTREFSEL_None);
	anaIn.interrupt(this, &PMDialPad::adcDone);
	anaIn.disable();
	intIn.input();
	intIn.fall(this, &PMDialPad::buttonPress);
	interruptMode();
}

PMDialPad::~PMDialPad() {
}

void PMDialPad::timeout() {
	if (status == MEASUREMENT) {
		status = CONVERT;
		anaIn.start();
	}
}

void PMDialPad::buttonPress() {
	if (status == IDLE) {
		measureMode();
		anaIn.start();
		convert_run = 0;
		status = CONVERT;
	}
}

void PMDialPad::adcDone() {
	if (status == CONVERT) {
		uint16_t res = anaIn.read_result_u16();
		keys[keymap(res)]++;
		// do SIMPLEKEYBOARD_NB_CONVERT_RUN runs, than lower the sampling freq
		if (convert_run++ >= SIMPLEKEYBOARD_NB_CONVERT_RUN) {
			convert_run = 0;
			uint8_t maxVal = 0;
			enum Key maxIdx = KEY_NONE;
			for (int i = 0; i < KEY_LIST_LEN; ++i) {
				if (keys[i] > maxVal) {
					maxVal = keys[i]; maxIdx = static_cast<enum Key>(i); keys[i] = 0;
				}
			}
			button = maxIdx;
			if (button == KEY_NONE) {
				interruptMode();
				status = IDLE;
			} else {
				status = MEASUREMENT;
				to.attach_us(this, &PMDialPad::timeout, RECHECK_TIME_US);
			}
		} else {
			status = MEASUREMENT;
			to.attach_us(this, &PMDialPad::timeout, DEBOUNCE_TIME_US);
		}
	}
}

void PMDialPad::measureMode() {
	intIn.disable_irq();
	scaleOut.write(1);
	scaleOut.output();
	anaIn.enable();
}

void PMDialPad::interruptMode() {
	anaIn.disable();
//	intIn.input();
	scaleOut.input();
	intIn.enable_irq();
}