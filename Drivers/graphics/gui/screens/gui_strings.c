/*
 * gui_strings.c
 *
 *  Created on: 27 ene. 2021
 *      Author: David
 */
#include "gui_strings.h"

// language indexes and LANGUAGE_COUNT defined in settings.h

const strings_t strings[LANGUAGE_COUNT] = {

    [lang_english] = {
      .boot_firstBoot = "First Boot!",
      .boot_Profile = "Profile",

      .main_error_noIron_Detected = "NO IRON DETECTED",
      .main_error_failsafe = "FAILSAFE MODE",
      .main_error_NTC_high = "NTC READ HIGH",
      .main_error_NTC_low = "NTC READ LOW",
      .main_error_VoltageLow = "VOLTAGE LOW",
      .main_mode_Sleep = "SLEEP",
      .main_mode_Sleep_xpos = 42,
      .main_mode_Standby = "STBY",
      .main_mode_Standby_xpos = 46,
      .main_mode_Boost = "BOOST",
      .main_mode_Boost_xpos = 41,

      .settings_IRON = "IRON",
      .settings_SYSTEM = "SYSTEM",
      .settings_DEBUG = "DEBUG",
      .settings_EDIT_TIPS = "EDIT TIPS",
      .settings_CALIBRATION = "CALIBRATION",
      .settings_EXIT = "EXIT",
#ifdef ENABLE_ADDONS
      .settings_ADDONS = "ADDONS/EXTRAS",
#endif

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
      .FUME_EXTRACTOR_Title         = "FUME EXT. CTL.",
      .FUME_EXTRACTOR_Mode          = "Mode",
      .FUME_EXTRACTOR_Modes         = { "DISABLED", "AUTO", "ALW. ON" },
      .FUME_EXTRACTOR_AfterRun      = "After Run",
      .FUME_EXTRACTOR_AfterRunUnit  = "s",
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
      .SWITCH_OFF_REMINDER_Title               = "SW OFF REMINDER",
      .SWITCH_OFF_REMINDER_EnableDisableOption = "Reminder",
      .SWITCH_OFF_REMINDER_InactivityDelay     = "Delay",
      .SWITCH_OFF_REMINDER_ReminderPeriod     = "Period",
      .SWITCH_OFF_REMINDER_TimeUnit            = "m",
      .SWITCH_OFF_REMINDER_BeepType            = "Beep len.",
      .SWITCH_OFF_REMINDER_BeepTypes           = {"SHORT" ,"MED." ,"LONG"},
#endif

      .IRON_Max_Temp = "Max temp",
      .IRON_Min_Temp = "Min temp",
      .IRON_Default_Temp = "Def temp",
      .IRON_Standby = "Standby",
      .IRON_Sleep = "Sleep",
      .IRON_Boost = "Boost",
      .IRON_Boost_Add = " Increase",
      .IRON_Wake_Mode = "Wake mode",
      .IRON_Shake_Filtering = " Filter",
      .IRON_Stand_Mode = " In stand",
      .IRON_Power = "Power",
      .IRON_Heater = "Heater",
      .IRON_ADC_Time = "ADC Time",
      .IRON_PWM_mul = "PWM mul.",
      .IRON_No_Iron = "No iron",
      .IRON_Error_Timeout = "Err time",
      .IRON_Error_Resume_Mode = " Resume",
      .IRON_FILTER_MENU = "FILTER MENU",
      .IRON_NTC_MENU = "NTC MENU",

      .FILTER_Filter = "Filter",
      .FILTER__Threshold = " Threshold",
      .FILTER__Count_limit = " Count limit",
      .FILTER__Step_down = " Step down",
      .FILTER__Min = " Min",
      .FILTER_Reset_limit = "Reset limit",

      .SYSTEM_Profile = "Profile",
      .SYSTEM_Boot = "Boot",
      .SYSTEM_Button_Wake = "Btn wake",
      .SYSTEM_Shake_Wake = "Shake wake",
      .SYSTEM_Encoder = "Encoder",
      .SYSTEM_Buzzer = "Buzzer",
      .SYSTEM_Temperature = "Temperature",
      .SYSTEM__Step = " Step",
      .SYSTEM__Big_Step = " Big step",
      .SYSTEM_Active_Detection = "Active det.",
      .SYSTEM_LVP = "LVP",
      .SYSTEM_Gui_Time = "Gui time",
      .SYSTEM_DEBUG = "DEBUG",
      .SYSTEM_RESET_MENU = "RESET MENU",
      .SYSTEM_Remember = "Remember last",
      .SYSTEM_RememberLastProfile = " Profile",
      .SYSTEM_RememberLastTip = " Sel. tip",
#ifdef HAS_BATTERY
      .SYSTEM_RememberLastTemp = " Set temp",
#endif

      .SYSTEM_DISPLAY_MENU = "DISPLAY",
#ifndef ST756
      .DISPLAY_ContrastOrBrightness = "Brightness",
#else
      .DISPLAY_ContrastOrBrightness = "Contrast",
#endif
      .DISPLAY_Offset = "Offset",
      .DISPLAY_Xflip = "X flip",
      .DISPLAY_Yflip = "Y flip",
      .DISPLAY_Ratio = "Ratio",
      .DISPLAY_Dim = "Dimmer",
      .DISPLAY_Dim_inSleep = " In sleep",

      .NTC_Enable_NTC = "Enable NTC",
      .NTC_Pull = "Pull",
      .NTC__Res = " Res",
      .NTC__Beta = " Beta",
      .NTC_NTC_Detect = "NTC Detect",
      .NTC__High = " High",
      .NTC__Low = " Low",

      .RESET_Reset_Settings = "Reset Settings",
      .RESET_Reset_Profile = "Reset Profile",
      .RESET_Reset_Profiles = "Reset Profiles",
      .RESET_Reset_All = "Reset All",
      .RESET_Reset_msg_settings_1 = "RESET SYSTEM",
      .RESET_Reset_msg_settings_2 = "SETTINGS?",
      .RESET_Reset_msg_profile_1 = "RESET CURRENT",
      .RESET_Reset_msg_profile_2 = "PROFILE?",
      .RESET_Reset_msg_profiles_1 = "RESET ALL",
      .RESET_Reset_msg_profiles_2 = "PROFILES?",
      .RESET_Reset_msg_all_1 = "PERFORM FULL",
      .RESET_Reset_msg_all_2 = "SYSTEM RESET?",

      .TIP_SETTINGS_Name = "Name",
      .TIP_SETTINGS_PID_kp = "PID Kp",
      .TIP_SETTINGS_PID_ki = "PID Ki",
      .TIP_SETTINGS_PID_kd = "PID Kd",
      .TIP_SETTINGS_PID_Imax = "PID Imax",
      .TIP_SETTINGS_PID_Imin = "PID Imin",
      .TIP_SETTINGS_COPY = "COPY",
      .TIP_SETTINGS_DELETE = "DELETE",

      .CAL_ZeroSet = "Zero set   ",       // Must be 11 chars long
      .CAL_Sampling = "Sampling   ",      // Must be 11 chars long
      .CAL_Captured = "Captured   ",      // Must be 11 chars long
      .CAL_Step = "CAL STEP:",
      .CAL_Wait = "WAIT...",
      .CAL_Measured = "MEASURED:",
      .CAL_Success = "SUCCESS!",
      .CAL_Failed = "FAILED!",
      .CAL_DELTA_HIGH_1 = "DELTA TOO HIGH!",
      .CAL_DELTA_HIGH_2 = "Adjust manually",
      .CAL_DELTA_HIGH_3 = "and try again",
      .CAL_Error = "ERROR DETECTED!",
      .CAL_Aborting = "Aborting...",

      ._Language = "Language",
      .__Temp = " Temp",
      .__Delay = " Delay",
      ._Cal_250 = "Cal 250\260C",
      ._Cal_400 = "Cal 400\260C",
      ._BACK = "BACK",
      ._SAVE = "SAVE",
      ._CANCEL = "CANCEL",
      ._STOP = "STOP",
      ._RESET = "RESET",
      ._START = "START",
      ._SETTINGS = "SETTINGS",
      ._ADD_NEW = "ADD NEW",

      .ERROR_RUNAWAY = "TEMP RUNAWAY",
      .ERROR_EXCEEDED = "EXCEEDED",
      .ERROR_UNKNOWN = "UNKNOWN ERROR",
      .ERROR_SYSTEM_HALTED = "SYSTEM HALTED",
      .ERROR_BTN_RESET = "Use btn to reset",

      .OffOn =       { "OFF", "ON" },
      .DownUp =      { "DOWN", "UP" },
      .WakeModes =   { "OFF", "STBY", "SLP", "ALL" },
      .wakeMode =    { "SHAKE", "STAND" },
      .encMode =     { "REV", "FWD" },
      .InitMode =    { "SLP", "STBY", "RUN" },
      .dimMode =     { "OFF", "SLP", "ALL" },
      .errMode =     { "SLP", "RUN", "LAST" },
    },

#ifdef USE_LANG_RUSSIAN
    [lang_russian] = {
      .boot_firstBoot = "Выберите",
      .boot_Profile = "Тип",

      .main_error_noIron_Detected = "ОТСОЕДИНЕН",
      .main_error_failsafe = "РЕЖИМ ЗАЩИТЫ",
      .main_error_NTC_high = "NTC ЗНАЧ ВЫС",
      .main_error_NTC_low = "NTC ЗНАЧ НИЗ",
      .main_error_VoltageLow = "НАПР.ЗАНИЖЕНО",
      .main_mode_Sleep = "ВЫКЛ",
      .main_mode_Sleep_xpos = 46,
      .main_mode_Standby = "СОН",
      .main_mode_Standby_xpos = 49,
      .main_mode_Boost = "БУСТ",
      .main_mode_Boost_xpos = 48,

      .settings_IRON = "ПАЯЛЬНИК",
      .settings_SYSTEM = "СТАНЦИЯ",
      .settings_DEBUG = "ОТЛАДКА",
      .settings_EDIT_TIPS = "КАРТРИДЖ",
      .settings_CALIBRATION = "КАЛИБРОВКА",
      .settings_EXIT = "ВЫХОД",
#ifdef ENABLE_ADDONS
      .settings_ADDONS = "ADDONS/EXTRAS",
#endif

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
      .FUME_EXTRACTOR_Title         = "FUME EXT. CTL.",
      .FUME_EXTRACTOR_Mode          = "Mode",
      .FUME_EXTRACTOR_Modes         = { "DISABLED", "AUTO", "ALW. ON" },
      .FUME_EXTRACTOR_AfterRun      = "After Run",
      .FUME_EXTRACTOR_AfterRunUnit  = "s",
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
      .SWITCH_OFF_REMINDER_Title               = "SW OFF REMINDER",
      .SWITCH_OFF_REMINDER_EnableDisableOption = "Reminder",
      .SWITCH_OFF_REMINDER_InactivityDelay     = "Delay",
      .SWITCH_OFF_REMINDER_ReminderPeriod     = "Period",
      .SWITCH_OFF_REMINDER_TimeUnit            = "m",
      .SWITCH_OFF_REMINDER_BeepType            = "Beep len.",
      .SWITCH_OFF_REMINDER_BeepTypes           = {"SHORT" ,"MED." ,"LONG"},
#endif

      .IRON_Max_Temp = "Максимум",
      .IRON_Min_Temp = "Минимум",
      .IRON_Default_Temp = "Стартовая",
      .IRON_Standby = "СОН",
      .IRON_Sleep = "ВЫКЛ",
      .IRON_Boost = "Буст",
      .IRON_Boost_Add = " Догрев",
      .IRON_Wake_Mode = "ДатчикСна",
      .IRON_Shake_Filtering = " Фильтр",
      .IRON_Stand_Mode = "Подставка",
      .IRON_Power = "Мощность",
      .IRON_Heater = "Ом жала",
      .IRON_ADC_Time = "АЦП Замер",
      .IRON_PWM_mul = "ШИМ множ.",
      .IRON_No_Iron = "ПорогОткл",
      .IRON_Error_Timeout = "Вр.Ошибки",
      .IRON_Error_Resume_Mode = " Возврат К",
      .IRON_FILTER_MENU = "ПАРАМ. ФИЛЬТРА",
      .IRON_NTC_MENU = "ПАРАМ. NTC",

      .FILTER_Filter = "Фильтр",
      .FILTER__Threshold = " Порог",
      .FILTER__Count_limit = " Отсчетов",
      .FILTER__Step_down = " Шаг вниз",
      .FILTER__Min = " Минимум",
      .FILTER_Reset_limit = "ПорогСброса",

      .SYSTEM_Profile = "Тип",
      .SYSTEM_Boot = "Старт с",
      .SYSTEM_Button_Wake = "Кнопка раб.",
      .SYSTEM_Shake_Wake = "Датчик раб.",
      .SYSTEM_Encoder = "Вращение",
      .SYSTEM_Buzzer = "Зуммер",
      .SYSTEM_Temperature = "Шкала темп.",
      .SYSTEM__Step = " Шаг",
      .SYSTEM__Big_Step = " Шаг быстр.",
      .SYSTEM_Active_Detection = "Проверка",
      .SYSTEM_LVP = "ПорогНапр.",
      .SYSTEM_Gui_Time = "Отрисовка",
      .SYSTEM_DEBUG = "Отладка",
      .SYSTEM_RESET_MENU = "МЕНЮ СБРОСА",
      .SYSTEM_Remember = "Remember last",
      .SYSTEM_RememberLastProfile = " Profile",
      .SYSTEM_RememberLastTip = " Sel. tip",
#ifdef HAS_BATTERY
      .SYSTEM_RememberLastTemp = " Set temp",
#endif

      .SYSTEM_DISPLAY_MENU = "DISPLAY",
#ifndef ST756
      .DISPLAY_ContrastOrBrightness = "Brightness",
#else
      .DISPLAY_ContrastOrBrightness = "Яркость",
#endif
      .DISPLAY_Offset = "Сдвиг",
      .DISPLAY_Xflip = "X Зерк.",
      .DISPLAY_Yflip = "Y Зерк.",
      .DISPLAY_Ratio = "Усиление",
      .DISPLAY_Dim = "Затемнение",
      .DISPLAY_Dim_inSleep = " Реж.Экрана",


      .NTC_Enable_NTC = "Внешний NTC",
      .NTC_Pull = "Подтяжка",
      .NTC__Res = " Сопр-ие",
      .NTC__Beta = "  Кривая",
      .NTC_NTC_Detect = "Автоопр.",
      .NTC__High = " Макс.",
      .NTC__Low = " Мин.",

      .RESET_Reset_Settings = "Сброс Настроек",
      .RESET_Reset_Profile = "Сброс Профиля",
      .RESET_Reset_Profiles = "Сброс Профилей",
      .RESET_Reset_All = "Общий сброс",
      .RESET_Reset_msg_settings_1 = "СБРОСИТЬ",
      .RESET_Reset_msg_settings_2 = "НАСТРОЙКИ?",
      .RESET_Reset_msg_profile_1 = "СБРОСИТЬ",
      .RESET_Reset_msg_profile_2 = "ТЕКУЩИЙ?",
      .RESET_Reset_msg_profiles_1 = "СБРОСИТЬ",
      .RESET_Reset_msg_profiles_2 = "ВСЕ?",
      .RESET_Reset_msg_all_1 = "ВЕРНУТЬ К",
      .RESET_Reset_msg_all_2 = "ЗАВОДСКИМ?",

      .TIP_SETTINGS_Name = "Название",
      .TIP_SETTINGS_PID_kp = "ПИД кП",
      .TIP_SETTINGS_PID_ki = "ПИД кИ",
      .TIP_SETTINGS_PID_kd = "ПИД кД",
      .TIP_SETTINGS_PID_Imax = "ПИД Имакс",
      .TIP_SETTINGS_PID_Imin = "ПИД Имин",
      .TIP_SETTINGS_COPY = "ДУБЛИРОВАТЬ",
      .TIP_SETTINGS_DELETE = "УДАЛИТЬ",

      .CAL_ZeroSet = "ХолСмещОУ  ",       // Must be 11 chars long
      .CAL_Sampling = "Замер...   ",      // Must be 11 chars long
      .CAL_Captured = "Записано   ",      // Must be 11 chars long
      .CAL_Step = "ШАГ:",
      .CAL_Wait = "ЖДИТЕ...",
      .CAL_Measured = "ЗАМЕР:",
      .CAL_Success = "УСПЕШНО!",
      .CAL_Failed = "НЕУДАЧА!",
      .CAL_DELTA_HIGH_1 = "РАЗНИЦА ВЕЛИКА",
      .CAL_DELTA_HIGH_2 = "задайте вручную",
      .CAL_DELTA_HIGH_3 = "и повторите",
      .CAL_Error = "ОШИБКА!",
      .CAL_Aborting = "выход...",

      ._Language = "Язык",
      .__Temp = " Нагрев",
      .__Delay = " Задержка",
      ._Cal_250 = "АЦП 250\260C",
      ._Cal_400 = "АЦП 400\260C",
      ._BACK = "НАЗАД",
      ._SAVE = "ЗАПИСЬ",
      ._CANCEL = "ОТМЕНА",
      ._STOP = "СТОП",
      ._RESET = "СБРОС",
      ._START = "НАЧАТЬ",
      ._SETTINGS = "ПАРАМЕТРЫ",
      ._ADD_NEW = "ДОБАВИТЬ",

      .ERROR_RUNAWAY = "УШЕЛ В РАЗНОС",
      .ERROR_EXCEEDED = "ПРЕВЫШЕНИЕ",
      .ERROR_UNKNOWN = "НЕИЗВ.ОШИБКА",
      .ERROR_SYSTEM_HALTED = "БЫЛИННЫЙ ОТКАЗ",
      .ERROR_BTN_RESET = "наж.для рестарта",

      .OffOn =       { "ВЫКЛ", "ВКЛ" },
      .DownUp =      { "ВНИЗ", "ВВЕРХ" },
      .WakeModes =   { "откл", "СОН", "ВЫКЛ", "все" },
      .wakeMode =    { "РУЧКА", "СТОЙКА" },
      .encMode =     { "ОБРАТНО", "ПРЯМО" },
      .InitMode =    { "ВЫКЛ", "СОН", "ПУСК" },
      .dimMode =     { "откл", "СОН", "все" },
      .errMode =     { "ВЫКЛ", "ПУСК", "ПРЕД" },

    },
#endif

#ifdef USE_LANG_SWEDISH
    [lang_swedish] = {
      .boot_firstBoot = "Första Start!",
      .boot_Profile = "Profil",

      .main_error_noIron_Detected = "KOLV EJ UPPTÄCKT",
      .main_error_failsafe = "FELSÄKERT LÄGE",
      .main_error_NTC_high = "NTC-VÄRDE HÖGT",
      .main_error_NTC_low = "NTC-VÄRDE LÅGT",
      .main_error_VoltageLow = "SPÄNNING LÅG",
      .main_mode_Sleep = "SOVER",
      .main_mode_Sleep_xpos = 40,
      .main_mode_Standby = "STBY",
      .main_mode_Standby_xpos = 46,
      .main_mode_Boost = "BOOST",
      .main_mode_Boost_xpos = 41,

      .settings_IRON = "LÖDKOLV",
      .settings_SYSTEM = "SYSTEM",
      .settings_DEBUG = "DEBUG",
      .settings_EDIT_TIPS = "SPETSAR",
      .settings_CALIBRATION = "KALIBRERING",
      .settings_EXIT = "AVSLUTA",
#ifdef ENABLE_ADDONS
      .settings_ADDONS = "ADDONS/EXTRAS",
#endif

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
      .FUME_EXTRACTOR_Title         = "FUME EXT. CTL.",
      .FUME_EXTRACTOR_Mode          = "Mode",
      .FUME_EXTRACTOR_Modes         = { "DISABLED", "AUTO", "ALW. ON" },
      .FUME_EXTRACTOR_AfterRun      = "After Run",
      .FUME_EXTRACTOR_AfterRunUnit  = "s",
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
      .SWITCH_OFF_REMINDER_Title               = "SW OFF REMINDER",
      .SWITCH_OFF_REMINDER_EnableDisableOption = "Reminder",
      .SWITCH_OFF_REMINDER_InactivityDelay     = "Delay",
      .SWITCH_OFF_REMINDER_ReminderPeriod     = "Period",
      .SWITCH_OFF_REMINDER_TimeUnit            = "m",
      .SWITCH_OFF_REMINDER_BeepType            = "Beep len.",
      .SWITCH_OFF_REMINDER_BeepTypes           = {"SHORT" ,"MED." ,"LONG"},
#endif

      .IRON_Max_Temp = "Max temp",
      .IRON_Min_Temp = "Min temp",
      .IRON_Default_Temp = "Starttemp",
      .IRON_Standby = "Standby",
      .IRON_Sleep = "Sovläge",
      .IRON_Boost = "Boost",
      .IRON_Boost_Add = " Tillägg",
      .IRON_Wake_Mode = "Väckmetod",
      .IRON_Shake_Filtering = " Filter",
      .IRON_Stand_Mode = "Ställ-läge",
      .IRON_Power = "Effekt",
      .IRON_Heater = "Element",
      .IRON_ADC_Time = "ADO Tid",
      .IRON_PWM_mul = "PWM mul.",
      .IRON_No_Iron = "Kolv ur",
      .IRON_Error_Timeout = "Feltid",
      .IRON_Error_Resume_Mode = " Återst.",
      .IRON_FILTER_MENU = "FILTER MENY",
      .IRON_NTC_MENU = "NTC MENY",

      .FILTER_Filter = "Filter",
      .FILTER__Threshold = " Tröskel",
      .FILTER__Count_limit = " Antalsgräns",
      .FILTER__Step_down = " Avbackning",
      .FILTER__Min = " Min",
      .FILTER_Reset_limit = "Resetgräns",

      .SYSTEM_Profile = "Profil",
      .SYSTEM_Boot = "Startläge",
      .SYSTEM_Button_Wake = "Knappväck",
      .SYSTEM_Shake_Wake = "Skakväck",
      .SYSTEM_Encoder = "Encoder",
      .SYSTEM_Buzzer = "Summer",
      .SYSTEM_Temperature = "Temperatur",
      .SYSTEM__Step = " Steg",
      .SYSTEM__Big_Step = " Storsteg",
      .SYSTEM_Active_Detection = "Aktiv avk.",
      .SYSTEM_LVP = "LSS",
      .SYSTEM_Gui_Time = "Gui-tid",
      .SYSTEM_DEBUG = "DEBUG",
      .SYSTEM_RESET_MENU = "ÅTERST. MENY",
      .SYSTEM_Remember = "Remember last",
      .SYSTEM_RememberLastProfile = " Profile",
      .SYSTEM_RememberLastTip = " Sel. tip",
#ifdef HAS_BATTERY
      .SYSTEM_RememberLastTemp = " Set temp",
#endif

      .SYSTEM_DISPLAY_MENU = "DISPLAY",
#ifndef ST756
      .DISPLAY_ContrastOrBrightness = "Brightness",
#else
      .DISPLAY_ContrastOrBrightness = "Kontrast",
#endif
      .DISPLAY_Offset = "Offset",
      .DISPLAY_Xflip = "X flip",
      .DISPLAY_Yflip = "Y flip",
      .DISPLAY_Ratio = "Ratio",
      .DISPLAY_Dim = "Dimmer",
      .DISPLAY_Dim_inSleep = " I sovläge",

      .NTC_Enable_NTC = "Aktivera NTC",
      .NTC_Pull = "Pull",
      .NTC__Res = " Res",
      .NTC__Beta = " Beta",
      .NTC_NTC_Detect = "NTC Auto-avk.",
      .NTC__High = " Hög",
      .NTC__Low = " Låg",

      .RESET_Reset_Settings = "Återst. Inst.",
      .RESET_Reset_Profile = "Återst. Profil",
      .RESET_Reset_Profiles = "Återst. Profiler",
      .RESET_Reset_All = "Återst. Allt",
      .RESET_Reset_msg_settings_1 = "ÅTERST. SYSTEM-",
      .RESET_Reset_msg_settings_2 = "INSTÄLLNINGAR?",
      .RESET_Reset_msg_profile_1 = "ÅTERST. AKTUELL",
      .RESET_Reset_msg_profile_2 = "PROFIL?",
      .RESET_Reset_msg_profiles_1 = "ÅTERST. ALLA",
      .RESET_Reset_msg_profiles_2 = "PROFILER?",
      .RESET_Reset_msg_all_1 = "UTFÖR TOTAL",
      .RESET_Reset_msg_all_2 = "SYSTEMÅTERST.?",

      .TIP_SETTINGS_Name = "Namn",
      .TIP_SETTINGS_PID_kd = "PID Kp",
      .TIP_SETTINGS_PID_ki = "PID Ki",
      .TIP_SETTINGS_PID_kp = "PID Kd",
      .TIP_SETTINGS_PID_Imax = "PID Imax",
      .TIP_SETTINGS_PID_Imin = "PID Imin",
      .TIP_SETTINGS_COPY = "KOPIERA",
      .TIP_SETTINGS_DELETE = "RADERA",

      .CAL_ZeroSet = "Nolläge    ",       // Must be 11 chars long
      .CAL_Sampling = "Mäter...   ",      // Must be 11 chars long
      .CAL_Captured = "Uppmätt    ",      // Must be 11 chars long
      .CAL_Step = "KAL STEG:",
      .CAL_Wait = "VÄNTA...",
      .CAL_Measured = "UPPMÄTT:",
      .CAL_Success = "KAL.OK!",
      .CAL_Failed = "KAL.FEL!",
      .CAL_DELTA_HIGH_1 = "DELTA FÖR HÖGT!",
      .CAL_DELTA_HIGH_2 = "Justera manuellt",
      .CAL_DELTA_HIGH_3 = "och försök igen",
      .CAL_Error = "FEL UPPTÄCKT!",
      .CAL_Aborting = "Avbryter...",

      ._Language = "Språk",
      .__Temp = " Temp",
      .__Delay = " Fördr.",
      ._Cal_250 = "Kal 250\260C",
      ._Cal_400 = "Kal 400\260C",
      ._BACK = "TILLBAKA",
      ._SAVE = "SPARA",
      ._CANCEL = "AVBRYT",
      ._STOP = "STOPP",
      ._RESET = "ÅTERST.",
      ._START = "START",
      ._SETTINGS = "INSTÄLLNINGAR",
      ._ADD_NEW = "LÄGG TILL",

      .ERROR_RUNAWAY = "TEMP AVVIKELSE",
      .ERROR_EXCEEDED = "ÖVERSKRIDET",
      .ERROR_UNKNOWN = "OKÄNT FEL",
      .ERROR_SYSTEM_HALTED = "SYSTEM STOPPAT",
      .ERROR_BTN_RESET = "Anv knapp för återst.",

      .OffOn =       { "AV", "PÅ" },
      .DownUp =      { "NER", "UPP" },
      .WakeModes =   { "AV", "STBY", "SOV", "ALLA" },
      .wakeMode =    { "SKAKA", "STÄLL" },
      .encMode =     { "OMVÄND", "NORMAL" },
      .InitMode =    { "SOV", "STBY", "KÖR" },
      .dimMode =     { "AV", "SOV", "ALLA" },
      .errMode =     { "SOV", "KÖR", "FRG" },
    },
#endif

#ifdef USE_LANG_GERMAN
    [lang_german] = {
      .boot_firstBoot = "Erster Start",
      .boot_Profile = "Profile",

      .main_error_noIron_Detected = "KEINE LÖTSPITZE",
      .main_error_failsafe = "FAILSAFE MODUS",
      .main_error_NTC_high = "NTC WERT HOCH",
      .main_error_NTC_low = "NTC WERT NIEDRIG",
      .main_error_VoltageLow = "SPANNUNG ZU NIEDRIG",
      .main_mode_Sleep = "SLEEP",
      .main_mode_Sleep_xpos = 42,
      .main_mode_Standby = "STBY",
      .main_mode_Standby_xpos = 46,
      .main_mode_Boost = "BOOST",
      .main_mode_Boost_xpos = 41,

      .settings_IRON = "LÖTKOLBEN",
      .settings_SYSTEM = "SYSTEM",
      .settings_DEBUG = "DEBUG",
      .settings_EDIT_TIPS = "LÖTSPITZEN EDIT",
      .settings_CALIBRATION = "KALIBRIERUNG",
      .settings_EXIT = "ENDE",
#ifdef ENABLE_ADDONS
      .settings_ADDONS = "ADDONS/EXTRAS",
#endif

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
      .FUME_EXTRACTOR_Title         = "FUME EXT. CTL.",
      .FUME_EXTRACTOR_Mode          = "Mode",
      .FUME_EXTRACTOR_Modes         = { "DISABLED", "AUTO", "ALW. ON" },
      .FUME_EXTRACTOR_AfterRun      = "After Run",
      .FUME_EXTRACTOR_AfterRunUnit  = "s",
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
      .SWITCH_OFF_REMINDER_Title               = "SW OFF REMINDER",
      .SWITCH_OFF_REMINDER_EnableDisableOption = "Reminder",
      .SWITCH_OFF_REMINDER_InactivityDelay     = "Delay",
      .SWITCH_OFF_REMINDER_ReminderPeriod     = "Period",
      .SWITCH_OFF_REMINDER_TimeUnit            = "m",
      .SWITCH_OFF_REMINDER_BeepType            = "Beep len.",
      .SWITCH_OFF_REMINDER_BeepTypes           = {"SHORT" ,"MED." ,"LONG"},
#endif

      .IRON_Max_Temp = "Max Temp",
      .IRON_Min_Temp = "Min Temp",
      .IRON_Default_Temp = "Usr Temp",
      .IRON_Standby = "Standby",
      .IRON_Sleep = "Sleep",
      .IRON_Boost = "Boost",
      .IRON_Boost_Add = " Erhöhen",
      .IRON_Wake_Mode = "Aufwachen",
      .IRON_Shake_Filtering = " Filter",
      .IRON_Stand_Mode = " Im Halter",
      .IRON_Power = "Power",
      .IRON_Heater = "Heizung",
      .IRON_ADC_Time = "ADC Zeit",
      .IRON_PWM_mul = "PWM mul.",
      .IRON_No_Iron = "Kein Kolben",
      .IRON_Error_Timeout = "Err time",
      .IRON_Error_Resume_Mode = " Fortsetz.",
      .IRON_FILTER_MENU = "FILTER MENÜ",
      .IRON_NTC_MENU = "NTC MENÜ",

      .FILTER_Filter = "Filter",
      .FILTER__Threshold = " Schwelle",
      .FILTER__Count_limit = " Grenze",
      .FILTER__Step_down = " Runter",
      .FILTER__Min = " Min",
      .FILTER_Reset_limit = "Limit Reset",

      .SYSTEM_Profile = "Profile",
      .SYSTEM_Boot = "Boot",
      .SYSTEM_Button_Wake = "Taster",
      .SYSTEM_Shake_Wake = "Bewegung",
      .SYSTEM_Encoder = "Encoder",
      .SYSTEM_Buzzer = "Buzzer",
      .SYSTEM_Temperature = "Temperatur",
      .SYSTEM__Step = " Schritt",
      .SYSTEM__Big_Step = " Sprung",
      .SYSTEM_Active_Detection = "Aktiv Erkenn.",
      .SYSTEM_LVP = "LVP",
      .SYSTEM_Gui_Time = "Gui Zeit",
      .SYSTEM_DEBUG = "DEBUG",
      .SYSTEM_RESET_MENU = "RESET MENÜ",
      .SYSTEM_Remember = "Remember last",
      .SYSTEM_RememberLastProfile = " Profile",
      .SYSTEM_RememberLastTip = " Sel. tip",
#ifdef HAS_BATTERY
      .SYSTEM_RememberLastTemp = " Set temp",
#endif

      .SYSTEM_DISPLAY_MENU = "DISPLAY",
#ifndef ST756
      .DISPLAY_ContrastOrBrightness = "Brightness",
#else
      .DISPLAY_ContrastOrBrightness = "Kontrast",
#endif
      .DISPLAY_Offset = "Versatz",
      .DISPLAY_Xflip = "X flip",
      .DISPLAY_Yflip = "Y flip",
      .DISPLAY_Ratio = "Ratio",
      .DISPLAY_Dim = "Dimmer",
      .DISPLAY_Dim_inSleep = " In sleep",

      .NTC_Enable_NTC = "NTC Anschalt",
      .NTC_Pull = "Pull",
      .NTC__Res = " Res",
      .NTC__Beta = " Beta",
      .NTC_NTC_Detect = "NTC Erkenn",
      .NTC__High = " Hoch",
      .NTC__Low = " Niedrig",

      .RESET_Reset_Settings = "Einstell. Reset",
      .RESET_Reset_Profile = "Profil Reset",
      .RESET_Reset_Profiles = "Profile Reset",
      .RESET_Reset_All = "Komplett Reset",
      .RESET_Reset_msg_settings_1 = "SYSTEM",
      .RESET_Reset_msg_settings_2 = "ZURÜCKSETZEN?",
      .RESET_Reset_msg_profile_1 = "AKTIVES PROFIL",
      .RESET_Reset_msg_profile_2 = "ZURÜCKSETZEN?",
      .RESET_Reset_msg_profiles_1 = "ALLE PROFILE",
      .RESET_Reset_msg_profiles_2 = "ZURÜCKSETZEN?",
      .RESET_Reset_msg_all_1 = "ALLES",
      .RESET_Reset_msg_all_2 = "ZURÜCKSETZEN?",

      .TIP_SETTINGS_Name = "Name",
      .TIP_SETTINGS_PID_kp = "PID Kp",
      .TIP_SETTINGS_PID_ki = "PID Ki",
      .TIP_SETTINGS_PID_kd = "PID Kd",
      .TIP_SETTINGS_PID_Imax = "PID Imax",
      .TIP_SETTINGS_PID_Imin = "PID Imin",
      .TIP_SETTINGS_COPY = "KOPIEREN",
      .TIP_SETTINGS_DELETE = "LÖSCHEN",

      .CAL_ZeroSet = "Zero set   ",       // Must be 11 chars long
      .CAL_Sampling = "Sampling   ",      // Must be 11 chars long
      .CAL_Captured = "Captured   ",      // Must be 11 chars long
      .CAL_Step = "CAL STEP:",
      .CAL_Wait = "WARTE...",
      .CAL_Measured = "GEMESSEN:",
      .CAL_Success = "ERFOLGREICH!",
      .CAL_Failed = "FEHLER!",
      .CAL_DELTA_HIGH_1 = "DELTA ZU HOCH!",
      .CAL_DELTA_HIGH_2 = "Manuell einstellen",
      .CAL_DELTA_HIGH_3 = "und erneut probieren",
      .CAL_Error = "FEHLER ERKANNT!",
      .CAL_Aborting = "Abbruch...",

      ._Language = "Sprache",
      .__Temp = " Temp",
      .__Delay = " Verzög.",
      ._Cal_250 = "Cal 250\260C",
      ._Cal_400 = "Cal 400\260C",
      ._BACK = "ZURÜCK",
      ._SAVE = "OK",
      ._CANCEL = "ABBRUCH",
      ._STOP = "STOP",
      ._RESET = "RESET",
      ._START = "START",
      ._SETTINGS = "EINSTELLUNGEN",
      ._ADD_NEW = "HINZUFÜGEN",

      .ERROR_RUNAWAY = "TEMP RUNAWAY",
      .ERROR_EXCEEDED = "ÜBERTROFFEN",
      .ERROR_UNKNOWN = "UNBEKANNTER FEHLER",
      .ERROR_SYSTEM_HALTED = "SYSTEM GESTOPPT",
      .ERROR_BTN_RESET = "Drücken für Reset",

      .OffOn =       { "AUS", "AN" },
      .DownUp =      { "RUNTER", "RAUF" },
      .WakeModes =   { "OFF", "STBY", "SLP", "ALL" },
      .wakeMode =    { "SHAKE", "STAND" },
      .encMode =     { "REVERSE", "NORMAL" },
      .InitMode =    { "SLP", "STBY", "RUN" },
      .dimMode =     { "OFF", "SLP", "ALL" },
      .errMode =     { "SLP", "RUN", "LAST" },
    },
#endif

#ifdef USE_LANG_TURKISH
    [lang_turkish] = {
      .boot_firstBoot = "İlk Önyükleme!",
      .boot_Profile = "Profil",

      .main_error_noIron_Detected = "UÇ ALGILANMADI",
      .main_error_failsafe = "GÜVENLİ MOD",
      .main_error_NTC_high = "NTC YÜKSEK OKUMA",
      .main_error_NTC_low = "NTC DÜŞÜK OKUMA",
      .main_error_VoltageLow = "VOLTAJ DÜŞÜK",
      .main_mode_Sleep = "UYKU",
      .main_mode_Sleep_xpos = 42,
      .main_mode_Standby = "BKLM",
      .main_mode_Standby_xpos = 46,
      .main_mode_Boost = "DARBE",
      .main_mode_Boost_xpos = 41,

      .settings_IRON = "ISITICI",
      .settings_SYSTEM = "SİSTEM",
      .settings_DEBUG = "HATA AYIKLAMA",
      .settings_EDIT_TIPS = "UÇ DÜZENLE",
      .settings_CALIBRATION = "KALİBRASYON",
      .settings_EXIT = "ÇIKIŞ",
#ifdef ENABLE_ADDONS
      .settings_ADDONS = "ADDONS/EXTRAS",
#endif

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
      .FUME_EXTRACTOR_Title         = "FUME EXT. CTL.",
      .FUME_EXTRACTOR_Mode          = "Mode",
      .FUME_EXTRACTOR_Modes         = { "DISABLED", "AUTO", "ALW. ON" },
      .FUME_EXTRACTOR_AfterRun      = "After Run",
      .FUME_EXTRACTOR_AfterRunUnit  = "s",
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
      .SWITCH_OFF_REMINDER_Title               = "SW OFF REMINDER",
      .SWITCH_OFF_REMINDER_EnableDisableOption = "Reminder",
      .SWITCH_OFF_REMINDER_InactivityDelay     = "Delay",
      .SWITCH_OFF_REMINDER_ReminderPeriod     = "Period",
      .SWITCH_OFF_REMINDER_TimeUnit            = "m",
      .SWITCH_OFF_REMINDER_BeepType            = "Beep len.",
      .SWITCH_OFF_REMINDER_BeepTypes           = {"SHORT" ,"MED." ,"LONG"},
#endif

      .IRON_Max_Temp = "Max Isı",
      .IRON_Min_Temp = "Min Isı",
      .IRON_Default_Temp = "İlk Isı",
      .IRON_Standby = "Bekleme",
      .IRON_Sleep = "Uyku",
      .IRON_Boost = "Darbe",
      .IRON_Boost_Add = " Arttırmak",
      .IRON_Wake_Mode = "Uyandırma",
      .IRON_Shake_Filtering = " Filtre",
      .IRON_Stand_Mode = " Stantta",
      .IRON_Power = "Güç",
      .IRON_Heater = "Isıtıcı",
      .IRON_ADC_Time = "ADC Zamanı",
      .IRON_PWM_mul = "PWM mul.",
      .IRON_No_Iron = "Uç yok",
      .IRON_Error_Timeout = "Hata zamanı",
      .IRON_Error_Resume_Mode = " Devam Et",
      .IRON_FILTER_MENU = "FİLTRE AYARLARI",
      .IRON_NTC_MENU = "NTC AYARLARI",

      .FILTER_Filter = "Filtre",
      .FILTER__Threshold = " Eşik",
      .FILTER__Count_limit = " Sayı limit",
      .FILTER__Step_down = " Düşürmek",
      .FILTER__Min = " Düşük",
      .FILTER_Reset_limit = "Sıfırlandı",

      .SYSTEM_Profile = "Profil",
      .SYSTEM_Boot = "Başlangıç",
      .SYSTEM_Button_Wake = "Düme Uyan",
      .SYSTEM_Shake_Wake = "Salla Uyan",
      .SYSTEM_Encoder = "Potans",
      .SYSTEM_Buzzer = "Buzzer",
      .SYSTEM_Temperature = "Sıcaklık",
      .SYSTEM__Step = " Adım",
      .SYSTEM__Big_Step = " Byk Adım",
      .SYSTEM_Active_Detection = "K.Yazı",
      .SYSTEM_LVP = "DVK",
      .SYSTEM_Gui_Time = "Gui zamanı",
      .SYSTEM_DEBUG = "Geliştirici",
      .SYSTEM_RESET_MENU = "MENÜ SIFIRLA",
      .SYSTEM_Remember = "Remember last",
      .SYSTEM_RememberLastProfile = " Profile",
      .SYSTEM_RememberLastTip = " Sel. tip",
#ifdef HAS_BATTERY
      .SYSTEM_RememberLastTemp = " Set temp",
#endif
      .SYSTEM_DISPLAY_MENU = "DISPLAY",

#ifndef ST756
      .DISPLAY_ContrastOrBrightness = "Brightness",
#else
      .DISPLAY_ContrastOrBrightness = "kontrast",
#endif
      .DISPLAY_Offset = "Dengele",
      .DISPLAY_Xflip = "X flip",
      .DISPLAY_Yflip = "Y flip",
      .DISPLAY_Ratio = "Ratio",
      .DISPLAY_Dim = "Karartma",
      .DISPLAY_Dim_inSleep = " Uykuda",

      .NTC_Enable_NTC = "NTC Etkin",
      .NTC_Pull = "Çek",
      .NTC__Res = " Omaj",
      .NTC__Beta = " Beta",
      .NTC_NTC_Detect = "NTC Tespit",
      .NTC__High = " Yüksek",
      .NTC__Low = " Düşük",

      .RESET_Reset_Settings = "Ayarlari Sil",
      .RESET_Reset_Profile = "Profili Sil",
      .RESET_Reset_Profiles = "Profilleri Sil",
      .RESET_Reset_All = "Hepsini Sil",
      .RESET_Reset_msg_settings_1 = "SİSTEMİ SIFIRLA",
      .RESET_Reset_msg_settings_2 = "AYARLAR?",
      .RESET_Reset_msg_profile_1 = "MEVCUT SIFIRLA",
      .RESET_Reset_msg_profile_2 = "PROFİL?",
      .RESET_Reset_msg_profiles_1 = "HEPSİNİ SIFIRLA",
      .RESET_Reset_msg_profiles_2 = "PROFİLLER?",
      .RESET_Reset_msg_all_1 = "TÜMÜNÜ YAPMAK",
      .RESET_Reset_msg_all_2 = "SİSTEM SIFIRLAMA?",

      .TIP_SETTINGS_Name = "isim",
      .TIP_SETTINGS_PID_kp = "PID Kp",
      .TIP_SETTINGS_PID_ki = "PID Ki",
      .TIP_SETTINGS_PID_kd = "PID Kd",
      .TIP_SETTINGS_PID_Imax = "PID Imax",
      .TIP_SETTINGS_PID_Imin = "PID Imin",
      .TIP_SETTINGS_COPY = "KOPYALA",
      .TIP_SETTINGS_DELETE = "SİL",

      .CAL_ZeroSet = "Sıfır seti ",       // Must be 11 chars long
      .CAL_Sampling = "Örnekleme  ",      // Must be 11 chars long
      .CAL_Captured = "Yakalandı  ",      // Must be 11 chars long
      .CAL_Step = "HESAP ADIM:",
      .CAL_Wait = "BEKLE...",
      .CAL_Measured = "ÖLÇÜLEN:",
      .CAL_Success = "BAŞARILI!",
      .CAL_Failed = "ARIZALI!",
      .CAL_DELTA_HIGH_1 = "DELTA YÜKSEK!",
      .CAL_DELTA_HIGH_2 = "Manuel ayarla",
      .CAL_DELTA_HIGH_3 = "Tekrar dene",
      .CAL_Error = "HATA TESPİTİ!",
      .CAL_Aborting = "İptal...",

      ._Language = "Dil",
      .__Temp = " Isı",
      .__Delay = " Gecikme",
      ._Cal_250 = "Cal 250\260C",
      ._Cal_400 = "Cal 400\260C",
      ._BACK = "GERİ",
      ._SAVE = "KAYDET",
      ._CANCEL = "ÇIK",
      ._STOP = "DURDUR",
      ._RESET = "RESET",
      ._START = "BAŞLAT",
      ._SETTINGS = "AYARLAR",
      ._ADD_NEW = "YENİ EKLE",

      .ERROR_RUNAWAY = "ISI TUTARSIZ",
      .ERROR_EXCEEDED = "AŞILDI",
      .ERROR_UNKNOWN = "BİLİNMEYEN HATA",
      .ERROR_SYSTEM_HALTED = "SİSTEM DURDU",
      .ERROR_BTN_RESET = "DÜME İLE SIFIRLA",

      .OffOn =       { "KPLI", "AÇIK" },
      .DownUp =      { "AŞAGI", "YUKARI" },
      .WakeModes =   { "KPLI", "BKLM", "UYKU", "TÜM" },
      .wakeMode =    { "SALLA", "SABİT" },
      .encMode =     { "TERS", "NORMAL" },
      .InitMode =    { "UYKU", "BKLM", "ÇLS" },
      .dimMode =     { "KPLI", "UYKU", "TÜM" },
      .errMode =     { "UYKU", "ÇLS", "SON" },
    },
#endif
};


char * const tempUnit[2]              = { "\260C", "\260F" };
char * const profileStr[NUM_PROFILES] = { "T12", "C245", "C210" };
char * const Langs[LANGUAGE_COUNT]    = {
                                            [lang_english] = "EN",

#ifdef USE_LANG_RUSSIAN
                                            [lang_russian] = "RU",
#endif
#ifdef USE_LANG_SWEDISH
                                            [lang_swedish] = "SWE",
#endif
#ifdef USE_LANG_GERMAN
                                            [lang_german]  = "GER",
#endif
#ifdef USE_LANG_TURKISH
                                            [lang_turkish] = "TR",
#endif
                                        };
