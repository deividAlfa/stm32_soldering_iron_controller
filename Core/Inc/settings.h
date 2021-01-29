/*
 * settings.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"
#include "pid.h"
#include "board.h"

#define ProfileSize		3		// Number of profiles
#define TipSize			10		// Number of tips for each profile
#define TipCharSize		5		// String size for each tip name

#define SYSTEM_VERSION	"Ver: 2021-13-01"
#define FW_Version 		20211301	 										// Change this if you change the struct below to prevent people getting out of sync
#define StoreSize 		2 											// In KB
#define FLASH_ADDR 		(0x8000000 + ((FLASH_SZ-StoreSize)*1024))	// Last 2KB flash (Minimum erase size, page size=2KB)

enum{
	wakeInputmode_shake	= 0,
	wakeInputmode_stand	= 1,

	wakeButton_Off	= 0,
	wakeButton_On	= 1,

	source_wakeInput	= 0,
	source_wakeButton	= 1,

	failureState_Off	= 0,
	failureState_On		= 1,

	no_update	= 0,
	needs_update		= 1,

	runaway_ok		= 0,
	runaway_25		= 1,
	runaway_50		= 2,
	runaway_75		= 3,
	runaway_100		= 4,
	runaway_500		= 5,

	runaway_triggered 	= 1,

	notPresent		= 0,
	isPresent		= 1,

	debug_Off		= 0,
	debug_On		= 1,

	calibration_Off	= 0,
	calibration_On	= 1,

	saveKeepingProfiles	= 0,
	saveWipingProfiles	= 1,

	setup_Off		= 0,
	setup_On		= 1,

	encoder_normal	= 0,
	encoder_reverse	= 1,

	mode_Celsius	= 0,
	mode_Farenheit	= 1,

	mode_sleep		= 0,
	mode_run		= 1,

	initialized		= 0,
	notInitialized	= 1,

	filter_avg		= 0,
	filter_ema		= 1,
	filter_dema		= 2,

	noForceMode		= 0,
	forceMode		= 1,

	buzzer_Off		= 0,
	buzzer_On		= 1,

	profile_T12		= 0,
	profile_C245	= 1,
	profile_C210	= 2,
	profile_None	= 0xff,
};


typedef struct ironSettings_t{
	uint16_t Temperature;
	uint16_t Time;
} ironSettings_t;

typedef struct tipData {
	uint16_t calADC_At_200;
	uint16_t calADC_At_300;
	uint16_t calADC_At_400;
	char name[TipCharSize];
	pid_values_t PID;
}tipData;

__attribute__ ((aligned (4))) typedef struct{
	tipData tip[TipSize];
	bool initialized;
	uint8_t tempUnit;
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t UserSetTemperature;
	uint16_t sleepTimeout;
	uint16_t pwmPeriod;
	uint16_t pwmDelay;
	uint16_t noIronValue;
	uint8_t filterMode;
	uint8_t filterCoef;
	uint16_t PIDTime;
}profile_t;

__attribute__ ((aligned (4))) typedef struct{
	uint32_t version;				// Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	uint8_t OledOffset;
	uint16_t noIronDelay;
	uint16_t guiUpdateDelay;
	uint8_t currentProfile;
	uint8_t saveSettingsDelay;
	uint8_t initMode;
	uint8_t tempStep;
	bool tempUnit;
	bool buzzerMode;
	bool wakeOnButton;
	bool WakeInputMode;
	bool EncoderInvert;
	bool initialized;			// Always 1 if flash is erased
}settings_t;

typedef struct{
	settings_t settings;
	uint32_t settingsChecksum;
	profile_t Profile[ProfileSize];
	uint32_t ProfileChecksum[ProfileSize];
}flashSettings_t;


typedef struct{
	settings_t settings;
	uint32_t settingsChecksum;
	profile_t Profile;
	uint32_t ProfileChecksum;
	bool setupMode;
}systemSettings_t;

extern systemSettings_t systemSettings;
extern flashSettings_t* flashSettings;

void Diag_init(void);
void saveSettings(bool wipeAllProfileData);
void restoreSettings();
uint32_t ChecksumSettings(settings_t* settings);
uint32_t ChecksumProfile(profile_t* profile);
void resetSystemSettings(void);
void resetCurrentProfile(void);
void storeTipData(uint8_t tip);
void loadProfile(uint8_t tip);

#endif /* SETTINGS_H_ */
