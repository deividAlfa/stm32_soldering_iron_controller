#!/usr/bin/env bash
set -euo pipefail

# STM32 Soldering Iron Controller — build script
# Replaces CubeMX by generating board-specific init files, then compiles.
# Works inside Docker or natively (auto-fetches HAL/CMSIS to /tmp).

BOARD="${1:-${BOARD:-KSGER_v3}}"
OLED="${2:-${OLED:-${DISPLAY:-SSD1306}}}"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

ALL_BOARDS="KSGER_v1.5 KSGER_v2 KSGER_v3 Quicko_072 Quicko_103 T12_958_v2"

# ---------------------------------------------------------------------------
# Detect native vs Docker
# ---------------------------------------------------------------------------
if [ -d "/src" ] && [ -d "/work" ]; then
  # Running inside Docker
  SRC="${SRC:-/src}"
  OUT="${OUT:-/output}"
  WORK="${WORK:-/work}"
  HAL_F1="${HAL_F1:-/opt/STM32CubeF1}"
  HAL_F0="${HAL_F0:-/opt/STM32CubeF0}"
else
  # Running natively
  SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
  SRC="${SRC:-$(cd "$SCRIPT_DIR/.." && pwd)}"
  OUT="${OUT:-$SRC/build}"
  WORK="${WORK:-/tmp/stm32-build}"
  HAL_F1="${HAL_F1:-/tmp/STM32CubeF1}"
  HAL_F0="${HAL_F0:-/tmp/STM32CubeF0}"
fi

# ---------------------------------------------------------------------------
# Board property lookups (bash 3.2 compatible — no associative arrays)
# ---------------------------------------------------------------------------
board_dir() {
  case "$1" in
    KSGER_v1.5) echo "BOARDS/KSGER/v1.5/STM32F103" ;;
    KSGER_v2)   echo "BOARDS/KSGER/v2/STM32F101" ;;
    KSGER_v3)   echo "BOARDS/KSGER/v3/STM32F101" ;;
    Quicko_072) echo "BOARDS/Quicko/STM32F072" ;;
    Quicko_103) echo "BOARDS/Quicko/STM32F103" ;;
    T12_958_v2) echo "BOARDS/Quecoo/T12-958_v2/STM32F103" ;;
    *) echo "" ;;
  esac
}

board_mcu() {
  case "$1" in
    KSGER_v1.5) echo "STM32F103CBTx" ;;
    KSGER_v2)   echo "STM32F101CBTx" ;;
    KSGER_v3)   echo "STM32F101CBTx" ;;
    Quicko_072) echo "STM32F072CBTx" ;;
    Quicko_103) echo "STM32F103CBTx" ;;
    T12_958_v2) echo "STM32F103CBTx" ;;
    *) echo "" ;;
  esac
}

board_family() {
  case "$1" in
    Quicko_072) echo "F0" ;;
    *)          echo "F1" ;;
  esac
}

board_defines() {
  case "$1" in
    KSGER_v1.5) echo "STM32F103xB" ;;
    KSGER_v2)   echo "STM32F101xB" ;;
    KSGER_v3)   echo "STM32F101xB" ;;
    Quicko_072) echo "STM32F072xB" ;;
    Quicko_103) echo "STM32F103xB" ;;
    T12_958_v2) echo "STM32F103xB" ;;
    *) echo "" ;;
  esac
}

board_cpu() {
  case "$1" in
    Quicko_072) echo "cortex-m0" ;;
    *)          echo "cortex-m3" ;;
  esac
}

# ---------------------------------------------------------------------------
# Fetch HAL/CMSIS if not present (native builds)
# ---------------------------------------------------------------------------
fetch_hal() {
  local family=$1
  if [ "$family" = "F1" ] && [ ! -d "$HAL_F1/Drivers/STM32F1xx_HAL_Driver/Src" ]; then
    HAL_F1=/tmp/STM32CubeF1
    echo ">> Fetching STM32CubeF1 HAL/CMSIS..."
    git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF1.git "$HAL_F1" 2>/dev/null || true
    cd "$HAL_F1"
    git submodule update --init --depth 1 Drivers/CMSIS/Device/ST/STM32F1xx Drivers/STM32F1xx_HAL_Driver
    cd - >/dev/null
  fi
  if [ "$family" = "F0" ] && [ ! -d "$HAL_F0/Drivers/STM32F0xx_HAL_Driver/Src" ]; then
    HAL_F0=/tmp/STM32CubeF0
    echo ">> Fetching STM32CubeF0 HAL/CMSIS..."
    git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF0.git "$HAL_F0" 2>/dev/null || true
    cd "$HAL_F0"
    git submodule update --init --depth 1 Drivers/CMSIS/Device/ST/STM32F0xx Drivers/STM32F0xx_HAL_Driver
    cd - >/dev/null
  fi
}

# ---------------------------------------------------------------------------
# Setup working directory
# ---------------------------------------------------------------------------
setup_workdir() {
  local board=$1
  local bdir family
  bdir="$(board_dir "$board")"
  family="$(board_family "$board")"

  rm -rf "$WORK"
  mkdir -p "$WORK"

  # Copy source tree
  cp -a "$SRC"/Core "$WORK"/
  cp -a "$SRC"/Drivers "$WORK"/
  cp -a "$SRC"/_STM32F*.ld "$WORK"/ 2>/dev/null || true

  # Overlay board-specific files
  cp -a "$SRC/$bdir"/.cproject "$WORK"/ 2>/dev/null || true
  cp -a "$SRC/$bdir"/.project  "$WORK"/ 2>/dev/null || true
  cp -a "$SRC/$bdir"/Core/Inc/* "$WORK"/Core/Inc/ 2>/dev/null || true
  cp -a "$SRC/$bdir"/Core/Src/* "$WORK"/Core/Src/ 2>/dev/null || true
  mkdir -p "$WORK/Core/Startup"
  cp -a "$SRC/$bdir"/Core/Startup/* "$WORK"/Core/Startup/ 2>/dev/null || true

  # Copy HAL/CMSIS
  if [ "$family" = "F1" ]; then
    mkdir -p "$WORK/Drivers/CMSIS/Device/ST/STM32F1xx/Include"
    mkdir -p "$WORK/Drivers/CMSIS/Include"
    mkdir -p "$WORK/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy"
    mkdir -p "$WORK/Drivers/STM32F1xx_HAL_Driver/Src"
    cp "$HAL_F1"/Drivers/CMSIS/Device/ST/STM32F1xx/Include/*.h "$WORK/Drivers/CMSIS/Device/ST/STM32F1xx/Include/"
    cp "$HAL_F1"/Drivers/CMSIS/Include/*.h "$WORK/Drivers/CMSIS/Include/"
    cp "$HAL_F1"/Drivers/STM32F1xx_HAL_Driver/Inc/*.h "$WORK/Drivers/STM32F1xx_HAL_Driver/Inc/"
    cp "$HAL_F1"/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy/*.h "$WORK/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy/"
    cp "$HAL_F1"/Drivers/STM32F1xx_HAL_Driver/Src/*.c "$WORK/Drivers/STM32F1xx_HAL_Driver/Src/"
  else
    mkdir -p "$WORK/Drivers/CMSIS/Device/ST/STM32F0xx/Include"
    mkdir -p "$WORK/Drivers/CMSIS/Include"
    mkdir -p "$WORK/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy"
    mkdir -p "$WORK/Drivers/STM32F0xx_HAL_Driver/Src"
    cp "$HAL_F0"/Drivers/CMSIS/Device/ST/STM32F0xx/Include/*.h "$WORK/Drivers/CMSIS/Device/ST/STM32F0xx/Include/"
    cp "$HAL_F0"/Drivers/CMSIS/Include/*.h "$WORK/Drivers/CMSIS/Include/"
    cp "$HAL_F0"/Drivers/STM32F0xx_HAL_Driver/Inc/*.h "$WORK/Drivers/STM32F0xx_HAL_Driver/Inc/"
    cp "$HAL_F0"/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy/*.h "$WORK/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy/"
    cp "$HAL_F0"/Drivers/STM32F0xx_HAL_Driver/Src/*.c "$WORK/Drivers/STM32F0xx_HAL_Driver/Src/"
  fi
}

# ===========================================================================
# CubeMX-equivalent file generators (per-board)
# ===========================================================================

# ---------------------------------------------------------------------------
# Common: stm32f1xx_hal_conf.h — same for all F1 boards
# ---------------------------------------------------------------------------
gen_hal_conf_f1() {
  cat > "$WORK/Core/Inc/stm32f1xx_hal_conf.h" <<'HALCONF'
#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED

#if !defined(HSE_VALUE)
#define HSE_VALUE 8000000U
#endif
#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT 100U
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE 8000000U
#endif
#if !defined(LSI_VALUE)
#define LSI_VALUE 40000U
#endif
#if !defined(LSE_VALUE)
#define LSE_VALUE 32768U
#endif
#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT 5000U
#endif
#define VDD_VALUE                    3300U
#define TICK_INT_PRIORITY            15U
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              1U

#define USE_HAL_ADC_REGISTER_CALLBACKS    0U
#define USE_HAL_I2C_REGISTER_CALLBACKS    0U
#define USE_HAL_SPI_REGISTER_CALLBACKS    0U
#define USE_HAL_TIM_REGISTER_CALLBACKS    0U

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f1xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f1xx_hal_gpio.h"
#endif
#ifdef HAL_EXTI_MODULE_ENABLED
#include "stm32f1xx_hal_exti.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f1xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f1xx_hal_cortex.h"
#endif
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32f1xx_hal_adc.h"
#endif
#ifdef HAL_CRC_MODULE_ENABLED
#include "stm32f1xx_hal_crc.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f1xx_hal_flash.h"
#endif
#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32f1xx_hal_i2c.h"
#endif
#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32f1xx_hal_iwdg.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32f1xx_hal_pwr.h"
#endif
#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32f1xx_hal_spi.h"
#endif
#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32f1xx_hal_tim.h"
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif
#endif
HALCONF
}

# ---------------------------------------------------------------------------
# Common: system_stm32f1xx.c — same for all F1 boards
# ---------------------------------------------------------------------------
gen_system_f1() {
  cat > "$WORK/Core/Src/system_stm32f1xx.c" <<'SYSTEM'
#include "stm32f1xx.h"
#if !defined(HSE_VALUE)
#define HSE_VALUE 8000000U
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE 8000000U
#endif
#define VECT_TAB_OFFSET 0x00000000U
uint32_t SystemCoreClock = 36000000U;
const uint8_t AHBPrescTable[16U] = {0,0,0,0,0,0,0,0,1,2,3,4,6,7,8,9};
const uint8_t APBPrescTable[8U]  = {0,0,0,0,1,2,3,4};
void SystemInit(void) {
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
}
void SystemCoreClockUpdate(void) {
  uint32_t tmp = RCC->CFGR & RCC_CFGR_SWS;
  switch (tmp) {
    case 0x00U: SystemCoreClock = HSI_VALUE; break;
    case 0x04U: SystemCoreClock = HSE_VALUE; break;
    case 0x08U: {
      uint32_t pllmull = ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18U) + 2U;
      if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0x00U)
        SystemCoreClock = (HSI_VALUE >> 1U) * pllmull;
      else if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) != (uint32_t)RESET)
        SystemCoreClock = (HSE_VALUE >> 1U) * pllmull;
      else
        SystemCoreClock = HSE_VALUE * pllmull;
      break;
    }
    default: SystemCoreClock = HSI_VALUE; break;
  }
  SystemCoreClock >>= AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4U)];
}
SYSTEM
}

# ===========================================================================
# KSGER v3 (STM32F101, SPI2 display)
# ===========================================================================
gen_cubemx_KSGER_v3() {
  gen_hal_conf_f1
  gen_system_f1

  # --- main.h: GPIO pin labels ---
  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define WAKE_Pin          GPIO_PIN_0
#define WAKE_GPIO_Port    GPIOA
#define BUZ0_Pin          GPIO_PIN_1
#define BUZ0_GPIO_Port    GPIOA
#define BUZ1_Pin          GPIO_PIN_2
#define BUZ1_GPIO_Port    GPIOA
#define NTC_Pin           GPIO_PIN_5
#define NTC_GPIO_Port     GPIOA
#define PWM_Pin           GPIO_PIN_6
#define PWM_GPIO_Port     GPIOA
#define VIN_Pin           GPIO_PIN_0
#define VIN_GPIO_Port     GPIOB
#define TIP_Pin           GPIO_PIN_1
#define TIP_GPIO_Port     GPIOB
#define HW_SCL_Pin        GPIO_PIN_13
#define HW_SCL_GPIO_Port  GPIOB
#define HW_SDA_Pin        GPIO_PIN_15
#define HW_SDA_GPIO_Port  GPIOB
#define DISPLAY_RST_Pin   GPIO_PIN_8
#define DISPLAY_RST_GPIO_Port GPIOA
#define DISPLAY_DC_Pin    GPIO_PIN_9
#define DISPLAY_DC_GPIO_Port  GPIOA
#define ENC_SW_Pin        GPIO_PIN_15
#define ENC_SW_GPIO_Port  GPIOA
#define ENC_L_Pin         GPIO_PIN_3
#define ENC_L_GPIO_Port   GPIOB
#define ENC_R_Pin         GPIO_PIN_4
#define ENC_R_GPIO_Port   GPIOB

#ifdef __cplusplus
}
#endif
#endif
MAINH

  # --- main.c: peripheral init + SystemClock_Config ---
  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f1xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM4_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_SPI2_Init();
  MX_TIM4_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  RCC_PeriphCLKInitTypeDef p = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  o.HSIState = RCC_HSI_ON;
  o.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_ON;
  o.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  o.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV1;
  c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  p.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_5; sc.Rank = ADC_REGULAR_RANK_1; sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_8; sc.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_9; sc.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_SPI2_Init(void) {
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK) Error_Handler();
}

static void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  TIM_OC_InitTypeDef oc = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 359;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &ck) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &mc) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim3);
}

static void MX_TIM4_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 359;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &mc) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
  g.Pin = GPIO_PIN_0; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_1|GPIO_PIN_2; g.Mode = GPIO_MODE_OUTPUT_OD; g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_8|GPIO_PIN_9; g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_15; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_3|GPIO_PIN_4; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  # --- stm32f1xx_hal_msp.c ---
  cat > "$WORK/Core/Src/stm32f1xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi2_tx;

void HAL_MspInit(void) {
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_5; g.Mode = GPIO_MODE_ANALOG; HAL_GPIO_Init(GPIOA, &g);
    g.Pin = GPIO_PIN_0|GPIO_PIN_1; HAL_GPIO_Init(GPIOB, &g);
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
  GPIO_InitTypeDef g = {0};
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_13|GPIO_PIN_15;
    g.Mode = GPIO_MODE_AF_PP; g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &g);
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hspi, hdmatx, hdma_spi2_tx);
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_15);
    HAL_DMA_DeInit(hspi->hdmatx);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) { __HAL_RCC_TIM3_CLK_ENABLE(); }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM3) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_6; g.Mode = GPIO_MODE_AF_PP; g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) __HAL_RCC_TIM3_CLK_DISABLE();
  else if (h->Instance == TIM4) { __HAL_RCC_TIM4_CLK_DISABLE(); HAL_NVIC_DisableIRQ(TIM4_IRQn); }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ===========================================================================
# KSGER v2 (STM32F101, I2C2 display — no SPI)
# ===========================================================================
gen_cubemx_KSGER_v2() {
  gen_hal_conf_f1
  gen_system_f1

  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define EE_SCL_Pin         GPIO_PIN_0
#define EE_SCL_GPIO_Port   GPIOA
#define EE_SDA_Pin         GPIO_PIN_1
#define EE_SDA_GPIO_Port   GPIOA
#define TIP_Pin            GPIO_PIN_2
#define TIP_GPIO_Port      GPIOA
#define BUZ0_Pin           GPIO_PIN_3
#define BUZ0_GPIO_Port     GPIOA
#define BUZ1_Pin           GPIO_PIN_4
#define BUZ1_GPIO_Port     GPIOA
#define BUZ2_Pin           GPIO_PIN_5
#define BUZ2_GPIO_Port     GPIOA
#define WAKE_Pin           GPIO_PIN_6
#define WAKE_GPIO_Port     GPIOA
#define NTC_Pin            GPIO_PIN_7
#define NTC_GPIO_Port      GPIOA
#define PWM_Pin            GPIO_PIN_0
#define PWM_GPIO_Port      GPIOB
#define VIN_Pin            GPIO_PIN_1
#define VIN_GPIO_Port      GPIOB
#define HW_SCL_Pin         GPIO_PIN_10
#define HW_SCL_GPIO_Port   GPIOB
#define HW_SDA_Pin         GPIO_PIN_11
#define HW_SDA_GPIO_Port   GPIOB
#define SW_SDA_Pin         GPIO_PIN_12
#define SW_SDA_GPIO_Port   GPIOB
#define SW_SCL_Pin         GPIO_PIN_13
#define SW_SCL_GPIO_Port   GPIOB
#define ENC_R_Pin          GPIO_PIN_3
#define ENC_R_GPIO_Port    GPIOB
#define ENC_L_Pin          GPIO_PIN_4
#define ENC_L_GPIO_Port    GPIOB
#define ENC_SW_Pin         GPIO_PIN_15
#define ENC_SW_GPIO_Port   GPIOA

#ifdef __cplusplus
}
#endif
#endif
MAINH

  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f1xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM4_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_TIM4_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  RCC_PeriphCLKInitTypeDef p = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  o.HSIState = RCC_HSI_ON;
  o.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_ON;
  o.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  o.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV1;
  c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  p.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_2; sc.Rank = ADC_REGULAR_RANK_1; sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_7; sc.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_9; sc.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  TIM_OC_InitTypeDef oc = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 359;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &ck) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &mc) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim3);
}

static void MX_TIM4_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 359;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &mc) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOA, BUZ0_Pin|BUZ1_Pin|BUZ2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, SW_SDA_Pin|SW_SCL_Pin, GPIO_PIN_SET);

  g.Pin = WAKE_Pin|ENC_SW_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = ENC_R_Pin|ENC_L_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);

  g.Pin = BUZ0_Pin|BUZ1_Pin|BUZ2_Pin;
  g.Mode = GPIO_MODE_OUTPUT_OD;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = SW_SDA_Pin|SW_SCL_Pin;
  g.Mode = GPIO_MODE_OUTPUT_OD;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  cat > "$WORK/Core/Src/stm32f1xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc1;

void HAL_MspInit(void) {
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_2|GPIO_PIN_7;
    g.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &g);
    g.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &g);
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM3) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_0;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ===========================================================================
# KSGER v1.5 (STM32F103, SPI1 display)
# ===========================================================================
gen_cubemx_KSGER_v1_5() {
  gen_hal_conf_f1
  gen_system_f1

  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define DISPLAY_CS_Pin      GPIO_PIN_0
#define DISPLAY_CS_GPIO_Port GPIOA
#define TIP_Pin             GPIO_PIN_1
#define TIP_GPIO_Port       GPIOA
#define VREF_Pin            GPIO_PIN_2
#define VREF_GPIO_Port      GPIOA
#define NTC_Pin             GPIO_PIN_3
#define NTC_GPIO_Port       GPIOA
#define VIN_Pin             GPIO_PIN_4
#define VIN_GPIO_Port       GPIOA
#define HW_SCL_Pin          GPIO_PIN_5
#define HW_SCL_GPIO_Port    GPIOA
#define HW_SDA_Pin          GPIO_PIN_7
#define HW_SDA_GPIO_Port    GPIOA
#define DISPLAY_DC_Pin      GPIO_PIN_8
#define DISPLAY_DC_GPIO_Port GPIOA
#define DISPLAY_RST_Pin     GPIO_PIN_9
#define DISPLAY_RST_GPIO_Port GPIOA
#define ENC_R_Pin           GPIO_PIN_0
#define ENC_R_GPIO_Port     GPIOB
#define ENC_L_Pin           GPIO_PIN_1
#define ENC_L_GPIO_Port     GPIOB
#define ENC_SW_Pin          GPIO_PIN_5
#define ENC_SW_GPIO_Port    GPIOB
#define WAKE_Pin            GPIO_PIN_6
#define WAKE_GPIO_Port      GPIOB
#define BUZZER_Pin          GPIO_PIN_7
#define BUZZER_GPIO_Port    GPIOB
#define PWM_Pin             GPIO_PIN_8
#define PWM_GPIO_Port       GPIOB

#ifdef __cplusplus
}
#endif
#endif
MAINH

  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f1xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM4_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  RCC_PeriphCLKInitTypeDef p = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
  o.HSEState = RCC_HSE_ON;
  o.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_ON;
  o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  o.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV2;
  c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  p.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_1; sc.Rank = ADC_REGULAR_RANK_1; sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_3; sc.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_4; sc.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_SPI1_Init(void) {
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

static void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 639;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &mc) != HAL_OK) Error_Handler();
}

static void MX_TIM4_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_OC_InitTypeDef oc = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 639;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &ck) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &oc, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim4);
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, DISPLAY_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, DISPLAY_DC_Pin|DISPLAY_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, BUZZER_Pin, GPIO_PIN_RESET);

  g.Pin = ENC_SW_Pin|WAKE_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);

  g.Pin = ENC_R_Pin|ENC_L_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);

  g.Pin = DISPLAY_CS_Pin|DISPLAY_DC_Pin|DISPLAY_RST_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = BUZZER_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  cat > "$WORK/Core/Src/stm32f1xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi1_tx;

void HAL_MspInit(void) {
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
    g.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &g);
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
  GPIO_InitTypeDef g = {0};
  if (hspi->Instance == SPI1) {
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_5|GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &g);
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hspi, hdmatx, hdma_spi1_tx);
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if (hspi->Instance == SPI1) {
    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
    HAL_DMA_DeInit(hspi->hdmatx);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM4) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_8;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
  }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ===========================================================================
# Quicko F103 (STM32F103, SPI2 display)
# ===========================================================================
gen_cubemx_Quicko_103() {
  gen_hal_conf_f1
  gen_system_f1

  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define DISPLAY_CS_Pin      GPIO_PIN_13
#define DISPLAY_CS_GPIO_Port GPIOC
#define VREF_Pin            GPIO_PIN_1
#define VREF_GPIO_Port      GPIOA
#define NTC_Pin             GPIO_PIN_2
#define NTC_GPIO_Port       GPIOA
#define VIN_Pin             GPIO_PIN_3
#define VIN_GPIO_Port       GPIOA
#define TIP_Pin             GPIO_PIN_5
#define TIP_GPIO_Port       GPIOA
#define ENC_R_Pin           GPIO_PIN_8
#define ENC_R_GPIO_Port     GPIOA
#define ENC_L_Pin           GPIO_PIN_9
#define ENC_L_GPIO_Port     GPIOA
#define WAKE_Pin            GPIO_PIN_10
#define WAKE_GPIO_Port      GPIOA
#define ENC_SW_Pin          GPIO_PIN_11
#define ENC_SW_GPIO_Port    GPIOA
#define BUZZER_Pin          GPIO_PIN_12
#define BUZZER_GPIO_Port    GPIOA
#define PWM_DBG_Pin         GPIO_PIN_3
#define PWM_DBG_GPIO_Port   GPIOB
#define PWM_Pin             GPIO_PIN_7
#define PWM_GPIO_Port       GPIOB
#define DISPLAY_DC_Pin      GPIO_PIN_11
#define DISPLAY_DC_GPIO_Port GPIOB
#define DISPLAY_RST_Pin     GPIO_PIN_12
#define DISPLAY_RST_GPIO_Port GPIOB
#define HW_SCL_Pin          GPIO_PIN_13
#define HW_SCL_GPIO_Port    GPIOB
#define HW_SDA_Pin          GPIO_PIN_15
#define HW_SDA_GPIO_Port    GPIOB

#ifdef __cplusplus
}
#endif
#endif
MAINH

  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f1xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM4_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_SPI2_Init();
  MX_TIM4_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  RCC_PeriphCLKInitTypeDef p = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
  o.HSEState = RCC_HSE_ON;
  o.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_ON;
  o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  o.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV2;
  c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  p.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_2; sc.Rank = ADC_REGULAR_RANK_1; sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_3; sc.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_5; sc.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_SPI2_Init(void) {
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK) Error_Handler();
}

static void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 639;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &mc) != HAL_OK) Error_Handler();
}

static void MX_TIM4_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_OC_InitTypeDef oc = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 63;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &ck) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &oc, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim4);
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, BUZZER_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, DISPLAY_DC_Pin|DISPLAY_RST_Pin|PWM_DBG_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, DISPLAY_CS_Pin, GPIO_PIN_RESET);

  g.Pin = ENC_R_Pin|ENC_L_Pin|WAKE_Pin|ENC_SW_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = BUZZER_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = DISPLAY_DC_Pin|DISPLAY_RST_Pin|PWM_DBG_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &g);

  g.Pin = DISPLAY_CS_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  cat > "$WORK/Core/Src/stm32f1xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi2_tx;

void HAL_MspInit(void) {
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5;
    g.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &g);
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
  GPIO_InitTypeDef g = {0};
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_13|GPIO_PIN_15;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &g);
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hspi, hdmatx, hdma_spi2_tx);
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_15);
    HAL_DMA_DeInit(hspi->hdmatx);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM4) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
  }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ===========================================================================
# T12-958 v2 (STM32F103)
# ===========================================================================
gen_cubemx_T12_958_v2() {
  gen_hal_conf_f1
  gen_system_f1

  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define SW_SDA_Pin          GPIO_PIN_0
#define SW_SDA_GPIO_Port    GPIOA
#define SW_SCL_Pin          GPIO_PIN_1
#define SW_SCL_GPIO_Port    GPIOA
#define DISPLAY_DC_Pin      GPIO_PIN_2
#define DISPLAY_DC_GPIO_Port GPIOA
#define DISPLAY_RST_Pin     GPIO_PIN_3
#define DISPLAY_RST_GPIO_Port GPIOA
#define VIN_Pin             GPIO_PIN_4
#define VIN_GPIO_Port       GPIOA
#define WAKE_Pin            GPIO_PIN_5
#define WAKE_GPIO_Port      GPIOA
#define NTC_Pin             GPIO_PIN_6
#define NTC_GPIO_Port       GPIOA
#define PWM_Pin             GPIO_PIN_7
#define PWM_GPIO_Port       GPIOA
#define TIP_Pin             GPIO_PIN_0
#define TIP_GPIO_Port       GPIOB
#define ENC_R_Pin           GPIO_PIN_5
#define ENC_R_GPIO_Port     GPIOB
#define ENC_L_Pin           GPIO_PIN_6
#define ENC_L_GPIO_Port     GPIOB
#define ENC_SW_Pin          GPIO_PIN_7
#define ENC_SW_GPIO_Port    GPIOB
#define BUZ_Pin             GPIO_PIN_9
#define BUZ_GPIO_Port       GPIOB
#define STAND_Pin           GPIO_PIN_1
#define STAND_GPIO_Port     GPIOD

#ifdef __cplusplus
}
#endif
#endif
MAINH

  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f1xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM4_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_CRC_Init();
  MX_TIM4_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  RCC_PeriphCLKInitTypeDef p = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  o.HSIState = RCC_HSI_ON;
  o.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_ON;
  o.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  o.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV1;
  c.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  p.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_4; sc.Rank = ADC_REGULAR_RANK_1; sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_6; sc.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_8; sc.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  TIM_OC_InitTypeDef oc = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 359;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &ck) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &mc) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim3);
}

static void MX_TIM4_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 359;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_UPDATE;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &mc) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, SW_SDA_Pin|SW_SCL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, DISPLAY_DC_Pin|DISPLAY_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, BUZ_Pin, GPIO_PIN_RESET);

  g.Pin = WAKE_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = ENC_R_Pin|ENC_L_Pin|ENC_SW_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &g);

  g.Pin = STAND_Pin;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STAND_GPIO_Port, &g);

  g.Pin = SW_SDA_Pin|SW_SCL_Pin|DISPLAY_DC_Pin|DISPLAY_RST_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &g);

  g.Pin = BUZ_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  cat > "$WORK/Core/Src/stm32f1xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc1;

void HAL_MspInit(void) {
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_4|GPIO_PIN_6;
    g.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &g);
    g.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &g);
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM3) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
  else if (h->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ---------------------------------------------------------------------------
# Common: stm32f0xx_hal_conf.h — for all F0 boards
# ---------------------------------------------------------------------------
gen_hal_conf_f0() {
  cat > "$WORK/Core/Inc/stm32f0xx_hal_conf.h" <<'HALCONF'
#ifndef __STM32F0xx_HAL_CONF_H
#define __STM32F0xx_HAL_CONF_H
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED

#if !defined(HSE_VALUE)
#define HSE_VALUE 8000000U
#endif
#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT 100U
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE 8000000U
#endif
#if !defined(HSI48_VALUE)
#define HSI48_VALUE 48000000U
#endif
#if !defined(HSI14_VALUE)
#define HSI14_VALUE 14000000U
#endif
#if !defined(LSI_VALUE)
#define LSI_VALUE 40000U
#endif
#if !defined(LSE_VALUE)
#define LSE_VALUE 32768U
#endif
#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT 5000U
#endif
#define VDD_VALUE                    3300U
#define TICK_INT_PRIORITY            3U
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              1U

#define USE_HAL_ADC_REGISTER_CALLBACKS    0U
#define USE_HAL_SPI_REGISTER_CALLBACKS    0U
#define USE_HAL_TIM_REGISTER_CALLBACKS    0U

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f0xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f0xx_hal_gpio.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f0xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f0xx_hal_cortex.h"
#endif
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32f0xx_hal_adc.h"
#include "stm32f0xx_hal_adc_ex.h"
#endif
#ifdef HAL_CRC_MODULE_ENABLED
#include "stm32f0xx_hal_crc.h"
#include "stm32f0xx_hal_crc_ex.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f0xx_hal_flash.h"
#include "stm32f0xx_hal_flash_ex.h"
#endif
#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32f0xx_hal_iwdg.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32f0xx_hal_pwr.h"
#include "stm32f0xx_hal_pwr_ex.h"
#endif
#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32f0xx_hal_spi.h"
#include "stm32f0xx_hal_spi_ex.h"
#endif
#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32f0xx_hal_tim.h"
#include "stm32f0xx_hal_tim_ex.h"
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif
#endif
HALCONF
}

# ---------------------------------------------------------------------------
# Common: system_stm32f0xx.c — for all F0 boards
# ---------------------------------------------------------------------------
gen_system_f0() {
  cat > "$WORK/Core/Src/system_stm32f0xx.c" <<'SYSTEM'
#include "stm32f0xx.h"
#if !defined(HSE_VALUE)
#define HSE_VALUE 8000000U
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE 8000000U
#endif
#if !defined(HSI48_VALUE)
#define HSI48_VALUE 48000000U
#endif
uint32_t SystemCoreClock = 48000000U;
const uint8_t AHBPrescTable[16U] = {0,0,0,0,0,0,0,0,1,2,3,4,6,7,8,9};
const uint8_t APBPrescTable[8U]  = {0,0,0,0,1,2,3,4};
void SystemInit(void) {
}
void SystemCoreClockUpdate(void) {
  uint32_t tmp = RCC->CFGR & RCC_CFGR_SWS;
  switch (tmp) {
    case 0x00U: SystemCoreClock = HSI_VALUE; break;
    case 0x04U: SystemCoreClock = HSE_VALUE; break;
    case 0x08U: {
      uint32_t pllmull = ((RCC->CFGR & RCC_CFGR_PLLMUL) >> 18U) + 2U;
      uint32_t predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1U;
      uint32_t pllsrc = RCC->CFGR & RCC_CFGR_PLLSRC;
      if (pllsrc == RCC_CFGR_PLLSRC_HSE_PREDIV)
        SystemCoreClock = (HSE_VALUE / predivfactor) * pllmull;
#if defined(RCC_CFGR_PLLSRC_HSI48_PREDIV)
      else if (pllsrc == RCC_CFGR_PLLSRC_HSI48_PREDIV)
        SystemCoreClock = (HSI48_VALUE / predivfactor) * pllmull;
#endif
      else
        SystemCoreClock = (HSI_VALUE / 2U) * pllmull;
      break;
    }
#if defined(RCC_CFGR_SWS_HSI48)
    case RCC_CFGR_SWS_HSI48: SystemCoreClock = HSI48_VALUE; break;
#endif
    default: SystemCoreClock = HSI_VALUE; break;
  }
  SystemCoreClock >>= AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4U)];
}
SYSTEM
}

# ===========================================================================
# Quicko F072 (STM32F072 — Cortex-M0, HSI48, SPI2 display)
# ===========================================================================
gen_cubemx_Quicko_072() {
  gen_hal_conf_f0
  gen_system_f0

  # --- main.h: GPIO pin labels ---
  cat > "$WORK/Core/Inc/main.h" <<'MAINH'
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f0xx_hal.h"
#include "user_main.h"

#ifdef DEBUG_ERROR
#define GET_MACRO(_0,_1,NAME,...) NAME
#define Error_Handler(...) GET_MACRO(_0,##__VA_ARGS__,Error_Handler1,Error_Handler0)()
#define Error_Handler0() _Error_Handler(__BASE_FILE__,__LINE__)
#define Error_Handler1(unused) _Error_Handler(char*file,int line)
void _Error_Handler(char*,int);
#endif

void Error_Handler(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#define DISPLAY_CS_Pin      GPIO_PIN_13
#define DISPLAY_CS_GPIO_Port GPIOC
#define VREF_Pin            GPIO_PIN_1
#define VREF_GPIO_Port      GPIOA
#define NTC_Pin             GPIO_PIN_2
#define NTC_GPIO_Port       GPIOA
#define VIN_Pin             GPIO_PIN_3
#define VIN_GPIO_Port       GPIOA
#define TIP_Pin             GPIO_PIN_5
#define TIP_GPIO_Port       GPIOA
#define ENC_R_Pin           GPIO_PIN_8
#define ENC_R_GPIO_Port     GPIOA
#define ENC_L_Pin           GPIO_PIN_9
#define ENC_L_GPIO_Port     GPIOA
#define WAKE_Pin            GPIO_PIN_10
#define WAKE_GPIO_Port      GPIOA
#define ENC_SW_Pin          GPIO_PIN_11
#define ENC_SW_GPIO_Port    GPIOA
#define BUZZER_Pin          GPIO_PIN_12
#define BUZZER_GPIO_Port    GPIOA
#define PWM_Pin             GPIO_PIN_7
#define PWM_GPIO_Port       GPIOB
#define DISPLAY_DC_Pin      GPIO_PIN_11
#define DISPLAY_DC_GPIO_Port GPIOB
#define DISPLAY_RST_Pin     GPIO_PIN_12
#define DISPLAY_RST_GPIO_Port GPIOB
#define HW_SCL_Pin          GPIO_PIN_13
#define HW_SCL_GPIO_Port    GPIOB
#define HW_SDA_Pin          GPIO_PIN_15
#define HW_SDA_GPIO_Port    GPIOB

#ifdef __cplusplus
}
#endif
#endif
MAINH

  # --- main.c: peripheral init + SystemClock_Config ---
  cat > "$WORK/Core/Src/main.c" <<'MAINC'
#include "main.h"
#include "stm32f0xx_hal.h"
#include "user_main.h"

ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;
CRC_HandleTypeDef hcrc;
IWDG_HandleTypeDef hiwdg;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim15;
TIM_HandleTypeDef htim17;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_IWDG_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM17_Init(void);
static void MX_TIM15_Init(void);
static void MX_TIM2_Init(void);

int main(void) {
  initBeforeMCUConfiguration();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_SPI2_Init();
  MX_TIM17_Init();
  MX_TIM15_Init();
  MX_IWDG_Init();
  MX_CRC_Init();
  MX_TIM2_Init();
  InitAfterMCUConfiguration();
  while (1) { mainCycle(); }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef o = {0};
  RCC_ClkInitTypeDef c = {0};
  o.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSI;
  o.HSI48State = RCC_HSI48_ON;
  o.LSIState = RCC_LSI_ON;
  o.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
  c.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  c.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  c.AHBCLKDivider = RCC_SYSCLK_DIV1;
  c.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
}

static void MX_ADC_Init(void) {
  ADC_ChannelConfTypeDef sc = {0};
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK) Error_Handler();
  sc.Rank = ADC_RANK_CHANNEL_NUMBER;
  sc.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  sc.Channel = ADC_CHANNEL_2;
  if (HAL_ADC_ConfigChannel(&hadc, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&hadc, &sc) != HAL_OK) Error_Handler();
  sc.Channel = ADC_CHANNEL_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sc) != HAL_OK) Error_Handler();
}

static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  if (HAL_CRC_Init(&hcrc) != HAL_OK) Error_Handler();
}

static void MX_IWDG_Init(void) {
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 624;
  hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) Error_Handler();
}

static void MX_SPI2_Init(void) {
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK) Error_Handler();
}

static void MX_TIM17_Init(void) {
  TIM_OC_InitTypeDef oc = {0};
  TIM_BreakDeadTimeConfigTypeDef bdt = {0};
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 479;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 19999;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_Init(&htim17) != HAL_OK) Error_Handler();
  oc.OCMode = TIM_OCMODE_PWM1;
  oc.Pulse = 1;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  oc.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  oc.OCFastMode = TIM_OCFAST_ENABLE;
  oc.OCIdleState = TIM_OCIDLESTATE_RESET;
  oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim17, &oc, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim17, TIM_CHANNEL_1);
  bdt.OffStateRunMode = TIM_OSSR_DISABLE;
  bdt.OffStateIDLEMode = TIM_OSSI_DISABLE;
  bdt.LockLevel = TIM_LOCKLEVEL_OFF;
  bdt.DeadTime = 0;
  bdt.BreakState = TIM_BREAK_DISABLE;
  bdt.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  bdt.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim17, &bdt) != HAL_OK) Error_Handler();
  HAL_TIM_MspPostInit(&htim17);
}

static void MX_TIM15_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 479;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 1999;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_RESET;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &mc) != HAL_OK) Error_Handler();
}

static void MX_TIM2_Init(void) {
  TIM_ClockConfigTypeDef ck = {0};
  TIM_MasterConfigTypeDef mc = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295U;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();
  ck.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &ck) != HAL_OK) Error_Handler();
  mc.MasterOutputTrigger = TIM_TRGO_RESET;
  mc.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &mc) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK) Error_Handler();
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOA, BUZZER_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, DISPLAY_DC_Pin|DISPLAY_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, DISPLAY_CS_Pin, GPIO_PIN_RESET);
  g.Pin = ENC_R_Pin|ENC_L_Pin|WAKE_Pin|ENC_SW_Pin;
  g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = BUZZER_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = DISPLAY_DC_Pin|DISPLAY_RST_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &g);
  g.Pin = DISPLAY_CS_Pin;
  g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &g);
}

void Error_Handler(void) {
  CrashErrorHandler(file, line);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
MAINC

  # --- stm32f0xx_hal_msp.c ---
  cat > "$WORK/Core/Src/stm32f0xx_hal_msp.c" <<'MSP'
#include "main.h"
extern DMA_HandleTypeDef hdma_adc;
extern DMA_HandleTypeDef hdma_spi2_tx;

void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
  GPIO_InitTypeDef g = {0};
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    g.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5;
    g.Mode = GPIO_MODE_ANALOG; g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &g);
    hdma_adc.Instance = DMA1_Channel1;
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc.Init.Mode = DMA_NORMAL;
    hdma_adc.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5);
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
  GPIO_InitTypeDef g = {0};
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_13|GPIO_PIN_15;
    g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH; g.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &g);
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(hspi, hdmatx, hdma_spi2_tx);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
  if (hspi->Instance == SPI2) {
    __HAL_RCC_SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_15);
    HAL_DMA_DeInit(hspi->hdmatx);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
  else if (h->Instance == TIM15) {
    __HAL_RCC_TIM15_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM15_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM15_IRQn);
  }
  else if (h->Instance == TIM17) {
    __HAL_RCC_TIM17_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* h) {
  GPIO_InitTypeDef g = {0};
  if (h->Instance == TIM17) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin = GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW; g.Alternate = GPIO_AF2_TIM17;
    HAL_GPIO_Init(GPIOB, &g);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* h) {
  if (h->Instance == TIM2) __HAL_RCC_TIM2_CLK_DISABLE();
  else if (h->Instance == TIM15) { __HAL_RCC_TIM15_CLK_DISABLE(); HAL_NVIC_DisableIRQ(TIM15_IRQn); }
  else if (h->Instance == TIM17) __HAL_RCC_TIM17_CLK_DISABLE();
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
  if (hcrc->Instance == CRC) {
    __HAL_RCC_CRC_CLK_DISABLE();
  }
}
MSP
}

# ===========================================================================
# CubeMX generator dispatcher (handles dots in board names)
# ===========================================================================
dispatch_cubemx() {
  local board=$1
  case "$board" in
    KSGER_v1.5) gen_cubemx_KSGER_v1_5 ;;
    KSGER_v2)   gen_cubemx_KSGER_v2 ;;
    KSGER_v3)   gen_cubemx_KSGER_v3 ;;
    Quicko_072) gen_cubemx_Quicko_072 ;;
    Quicko_103) gen_cubemx_Quicko_103 ;;
    T12_958_v2) gen_cubemx_T12_958_v2 ;;
    *) echo "!! No CubeMX generator for board: $board" >&2; exit 1 ;;
  esac
}

validate_generated_files() {
  local family=$1
  if [ "$family" = "F1" ]; then
    local msp="$WORK/Core/Src/stm32f1xx_hal_msp.c"
    if [ ! -f "$msp" ] || ! grep -q "HAL_CRC_MspInit" "$msp" || ! grep -q "__HAL_RCC_CRC_CLK_ENABLE" "$msp"; then
      echo "!! Generated F1 MSP is missing CRC clock init hooks." >&2
      exit 1
    fi
  fi
}

# ===========================================================================
# IRQ handlers normally generated by CubeMX into stm32f*_it.c
# ===========================================================================
augment_irq_handlers() {
  local board=$1
  local it_file=""

  case "$board" in
    KSGER_v1.5)
      it_file="$WORK/Core/Src/stm32f1xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF15'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim3;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void DMA1_Channel3_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

void TIM3_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim3);
}
ITF15
      fi
      ;;
    KSGER_v2)
      it_file="$WORK/Core/Src/stm32f1xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF12'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim4;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void TIM4_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim4);
}
ITF12
      fi
      ;;
    KSGER_v3)
      it_file="$WORK/Core/Src/stm32f1xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF1'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern TIM_HandleTypeDef htim4;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void DMA1_Channel5_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void TIM4_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim4);
}
ITF1
      fi
      ;;
    Quicko_103)
      it_file="$WORK/Core/Src/stm32f1xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF103'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern TIM_HandleTypeDef htim3;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void DMA1_Channel5_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void TIM3_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim3);
}
ITF103
      fi
      ;;
    T12_958_v2)
      it_file="$WORK/Core/Src/stm32f1xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF958'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim4;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void TIM4_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim4);
}
ITF958
      fi
      ;;
    Quicko_072)
      it_file="$WORK/Core/Src/stm32f0xx_it.c"
      [ -f "$it_file" ] || return 1
      if ! grep -q "DMA1_Channel1_IRQHandler" "$it_file"; then
        cat >> "$it_file" <<'ITF0'

/* Auto-generated IRQ handlers for non-CubeMX builds */
extern DMA_HandleTypeDef hdma_adc;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern TIM_HandleTypeDef htim15;

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_adc);
}

void DMA1_Channel4_5_6_7_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void TIM15_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim15);
}
ITF0
      fi
      ;;
  esac
}

# ===========================================================================
# Makefile generator
# ===========================================================================
gen_makefile() {
  local board=$1 display=$2
  local family cpu define
  family="$(board_family "$board")"
  cpu="$(board_cpu "$board")"
  define="$(board_defines "$board")"

  # Determine HAL sources and include paths per family
  local hal_prefix hal_inc cmsis_dev linker_glob startup_glob
  if [ "$family" = "F1" ]; then
    hal_prefix="stm32f1xx"
    hal_inc="-IDrivers/STM32F1xx_HAL_Driver/Inc -IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy"
    cmsis_dev="-IDrivers/CMSIS/Device/ST/STM32F1xx/Include"
    if [ "$define" = "STM32F103xB" ]; then
      linker_glob="_STM32F103*.ld"
    else
      linker_glob="_STM32F101*.ld"
    fi
    startup_glob="Core/Startup/*.s"
  else
    hal_prefix="stm32f0xx"
    hal_inc="-IDrivers/STM32F0xx_HAL_Driver/Inc -IDrivers/STM32F0xx_HAL_Driver/Inc/Legacy"
    cmsis_dev="-IDrivers/CMSIS/Device/ST/STM32F0xx/Include"
    linker_glob="_STM32F072*.ld"
    startup_glob="Core/Startup/*.s"
  fi

  # Find the linker script
  local ldscript
  ldscript=$(basename "$(ls $WORK/$linker_glob 2>/dev/null | head -1)")
  [ -z "$ldscript" ] && { echo "!! No linker script matching $linker_glob" >&2; exit 1; }

  # Find startup assembly
  local startup
  startup=$(ls $WORK/$startup_glob 2>/dev/null | head -1 | sed "s|$WORK/||")
  [ -z "$startup" ] && { echo "!! No startup file" >&2; exit 1; }

  # Collect C sources
  local c_sources
  c_sources=$(find "$WORK" -name '*.c' \
    -not -path '*/BOARDS/*' \
    -not -path '*/tools/*' \
    -not -path '*_template.c' \
    -not -path "*/STM32F*_HAL_Driver/Src/*" \
    | sed "s|$WORK/||" | sort)

  # Add only needed HAL driver sources (not templates)
  local hal_sources
  hal_sources=$(find "$WORK/Drivers" -path "*HAL_Driver/Src/${hal_prefix}_hal*.c" \
    -not -name '*_template.c' \
    | sed "s|$WORK/||" | sort)

  # Check for addon defines from board.h
  local addon_defs=""
  if grep -q "^#define ENABLE_ADDON_SWITCH_OFF_REMINDER" "$WORK/Core/Inc/board.h" 2>/dev/null; then
    addon_defs="$addon_defs -DENABLE_ADDON_SWITCH_OFF_REMINDER"
  fi
  if grep -q "^#define ENABLE_ADDON_FUME_EXTRACTOR" "$WORK/Core/Inc/board.h" 2>/dev/null; then
    addon_defs="$addon_defs -DENABLE_ADDON_FUME_EXTRACTOR"
  fi

  cat > "$WORK/Makefile" <<MKEOF
TARGET = STM32SolderingStation
BUILD_DIR = build
PREFIX = arm-none-eabi-
CC = \$(PREFIX)gcc
AS = \$(PREFIX)gcc -x assembler-with-cpp
CP = \$(PREFIX)objcopy
SZ = \$(PREFIX)size

C_SOURCES = \\
$(echo "$c_sources" | sed 's/$/ \\/')
$(echo "$hal_sources" | sed 's/$/ \\/' | sed '$ s/ \\$//')

ASM_SOURCES = $startup

MCU = -mcpu=${cpu} -mthumb
C_DEFS = -DUSE_HAL_DRIVER -D${define} -D${display}${addon_defs}
C_INCLUDES = -ICore/Inc $cmsis_dev -IDrivers/CMSIS/Include \\
  -IDrivers/generalIO -IDrivers/graphics -IDrivers/graphics/gui \\
  -IDrivers/graphics/gui/screens -IDrivers/graphics/u8g2 \\
  ${hal_inc} -IDrivers/addons

CFLAGS = \$(MCU) \$(C_DEFS) \$(C_INCLUDES) -Os -Wall \\
  -Wno-builtin-macro-redefined -ffunction-sections -fdata-sections \\
  -fstack-usage --specs=nano.specs -MMD -MP -MF"\$(@:%.o=%.d)"

ASFLAGS = \$(MCU) -Os -Wall -fdata-sections -ffunction-sections

LDSCRIPT = ${ldscript}
LDFLAGS = \$(MCU) -T\$(LDSCRIPT) -lc -lm -lnosys -Wl,--gc-sections \\
  --specs=nano.specs --specs=nosys.specs -Wl,-Map=\$(BUILD_DIR)/output.map

OBJECTS = \$(addprefix \$(BUILD_DIR)/,\$(notdir \$(C_SOURCES:.c=.o)))
vpath %.c \$(sort \$(dir \$(C_SOURCES)))
OBJECTS += \$(addprefix \$(BUILD_DIR)/,\$(notdir \$(ASM_SOURCES:.s=.o)))
vpath %.s \$(sort \$(dir \$(ASM_SOURCES)))

all: \$(BUILD_DIR)/\$(TARGET).bin

\$(BUILD_DIR)/%.o: %.c Makefile | \$(BUILD_DIR)
	\$(CC) -c \$(CFLAGS) \$< -o \$@

\$(BUILD_DIR)/%.o: %.s Makefile | \$(BUILD_DIR)
	\$(AS) -c \$(ASFLAGS) \$< -o \$@

\$(BUILD_DIR)/\$(TARGET).elf: \$(OBJECTS) Makefile
	\$(CC) \$(OBJECTS) \$(LDFLAGS) -o \$@
	\$(SZ) \$@

\$(BUILD_DIR)/\$(TARGET).bin: \$(BUILD_DIR)/\$(TARGET).elf
	\$(CP) -O binary -S \$< \$@

\$(BUILD_DIR):
	mkdir -p \$@

clean:
	rm -rf \$(BUILD_DIR)

.PHONY: all clean
-include \$(wildcard \$(BUILD_DIR)/*.d)
MKEOF
}

# ===========================================================================
# Build one target
# ===========================================================================
build_one() {
  local board=$1 display=$2
  local artifact_name="${3:-${board}_${display}}"
  local bdir mcu family
  bdir="$(board_dir "$board")"
  mcu="$(board_mcu "$board")"
  family="$(board_family "$board")"

  [ -z "$bdir" ] && { echo "!! Unknown board: $board"; echo "   Known: $ALL_BOARDS"; exit 1; }
  [ ! -d "$SRC/$bdir" ] && { echo "!! Board dir not found: $SRC/$bdir"; exit 1; }

  echo "========================================"
  echo "  Board:   $board"
  echo "  Display: $display"
  echo "  MCU:     $mcu"
  echo "========================================"

  fetch_hal "$family"
  setup_workdir "$board"

  # Generate CubeMX-equivalent files
  dispatch_cubemx "$board"
  validate_generated_files "$family"
  augment_irq_handlers "$board"

  # Generate Makefile
  gen_makefile "$board" "$display"

  # Build
  cd "$WORK"
  make -j"$JOBS" 2>&1 || return 1

  # Copy output
  mkdir -p "$OUT"
  cp "$WORK/build/STM32SolderingStation.bin" "$OUT/${artifact_name}.bin" || return 1
  echo ""
  echo ">> Built: $OUT/${artifact_name}.bin ($(stat -f%z "$OUT/${artifact_name}.bin" 2>/dev/null || stat -c%s "$OUT/${artifact_name}.bin" 2>/dev/null) bytes)"
  echo ""
}

build_zip() {
  local artifact_name=$1
  if ! command -v zip >/dev/null 2>&1; then
    echo "!! zip tool not found. Install 'zip' to build release artifacts." >&2
    return 1
  fi
  (
    cd "$OUT"
    rm -f "${artifact_name}.zip"
    zip -q "${artifact_name}.zip" "${artifact_name}.bin"
  ) || return 1
  echo ">> Packed: $OUT/${artifact_name}.zip"
}

build_matrix_all() {
  local failed=0
  while read -r b d; do
    [ -z "$b" ] && continue
    if ! build_one "$b" "$d"; then
      echo "!! FAILED: $b $d"
      failed=1
    fi
  done <<'ALLMATRIX'
KSGER_v1.5 SSD1306
KSGER_v1.5 ST7565
KSGER_v2 SSD1306
KSGER_v3 SSD1306
KSGER_v3 ST7565
Quicko_072 SSD1306
Quicko_072 ST7565
Quicko_103 SSD1306
Quicko_103 ST7565
T12_958_v2 SSD1306
T12_958_v2 ST7565
ALLMATRIX
  return "$failed"
}

build_matrix_release() {
  local failed=0
  while read -r b d name; do
    [ -z "$b" ] && continue
    if build_one "$b" "$d" "$name"; then
      build_zip "$name" || failed=1
    else
      echo "!! FAILED: $b $d ($name)"
      failed=1
    fi
  done <<'RELEASEMATRIX'
KSGER_v1.5 ST7565 KSGER_v1_5_LCD_ST7565
KSGER_v1.5 SSD1306 KSGER_v1_5_OLED
KSGER_v2 SSD1306 KSGER_v2_OLED
KSGER_v3 ST7565 KSGER_v3_LCD_ST7565
KSGER_v3 SSD1306 KSGER_v3_OLED
T12_958_v2 ST7565 Quecoo_T12-958_v2_LCD_ST7565
T12_958_v2 SSD1306 Quecoo_T12-958_v2_OLED
Quicko_072 ST7565 Quicko_STM32F072_LCD_ST7565
Quicko_072 SSD1306 Quicko_STM32F072_OLED
Quicko_103 ST7565 Quicko_STM32F103_LCD_ST7565
RELEASEMATRIX
  return "$failed"
}

# ===========================================================================
# Main
# ===========================================================================
if [ "$BOARD" = "all" ]; then
  build_matrix_all
elif [ "$BOARD" = "release" ]; then
  build_matrix_release
else
  build_one "$BOARD" "$OLED"
fi
