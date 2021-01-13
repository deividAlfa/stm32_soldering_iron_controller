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

#define ProfileSize		3		// Number of profiles
#define TipSize			10		// Number of tips for each profile
#define TipCharSize		5		// String size for each tip name

#define FW_Version 		12	 										// Change this if you change the struct below to prevent people getting out of sync
#define StoreSize 		2 											// In KB
#define FLASH_ADDR 		(0x8000000 + ((FLASH_SZ-StoreSize)*1024))	// Last 2KB flash (Minimum erase size, page size=2KB)

enum{

	Profile_T12		= 0,
	Profile_C245	= 1,
	Profile_C210	= 2,
	Profile_None	= 0xff,
};

extern char* profileStr[ProfileSize];

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

typedef struct{
	tipData tip[TipSize];
	bool notInitialized;
	ironSettings_t boost;
	ironSettings_t sleep;
	ironSettings_t standby;
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t UserSetTemperature;
	uint16_t pwmPeriod;
	uint16_t pwmDelay;
	uint16_t noIronValue;
}Profile_t;

typedef struct{
	uint8_t version;				// Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	uint8_t OledFix;
	uint16_t noIronDelay;
	uint16_t guiUpdateDelay;
	uint8_t currentProfile;
	uint8_t saveSettingsDelay;
	uint8_t initMode;
	bool tempUnit;
	bool buzzEnable;
	bool wakeOnButton;
	bool notInitialized;			// Always 1 if flash is erased
}settings_t;

typedef struct{
	settings_t settings;
	uint32_t settingsChecksum;
	Profile_t Profile[ProfileSize];
	uint32_t ProfileChecksum[ProfileSize];
}flashSettings_t;


typedef struct{
	settings_t settings;
	uint32_t settingsChecksum;
	Profile_t Profile;
	uint32_t ProfileChecksum;
	bool setupMode;
}systemSettings_t;

extern systemSettings_t systemSettings;
extern flashSettings_t* flashSettings;
void saveSettings(bool wipeAllProfileData);
void restoreSettings();
uint32_t ChecksumSettings(settings_t* settings);
uint32_t ChecksumProfile(Profile_t* profile);
void resetSystemSettings(void);
void resetCurrentProfile(void);
void storeTipData(uint8_t tip);
void loadProfile(uint8_t tip);

#endif /* SETTINGS_H_ */
