/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <err.h>
#include <debug.h>
#include <target.h>
#include <compiler.h>
#include <dload_util.h>
#include <sdhci_msm.h>
#if PON_VIB_SUPPORT
#include <smem.h>
#include <vibrator.h>
#include <board.h>
#endif

#include <smem.h>
#include <pm8x41_adc.h>
#include <pm8x41_hw.h>
#include <scm.h>

#if CHECK_BAT_VOLTAGE
#include <pm_fg_adc_usr.h>
#endif

#if VERIFIED_BOOT || VERIFIED_BOOT_2
#include <partition_parser.h>
#include <ab_partition_parser.h>
#endif

#define EXPAND(NAME) #NAME
#define TARGET(NAME) EXPAND(NAME)

#define BATTERY_MIN_VOLTAGE		3200000  //uv
#define PMIC_SLAVE_ID                   0x20000
#define BAT_IF_BAT_PRES_STATUS		0x1208
#define BAT_IF_INT_RT_STS		0x1210
#define BATT_INFO_VBATT_LSB		0x41A0
#define BATT_INFO_VBATT_MSB		0x41A1
#define ADC_V_DATA_LSB			0x248C0
#define ADC_V_DATA_MSB			0x248C1
#define BATT_VOLTAGE_NUMR		122070
#define QGAUGE_VOLTAGE_NUMR		194637
#define BATT_VOLTAGE_DENR		1000
#define INVALID  -1
static int vb_version = INVALID;

/*
 * default implementations of these routines, if the target code
 * chooses not to implement.
 */

__WEAK void target_early_init(void)
{
}

__WEAK void target_init(void)
{
}

__WEAK void *target_get_scratch_address(void)
{
    return (void *)(SCRATCH_ADDR);
}

__WEAK unsigned target_get_max_flash_size(void)
{
    return (120 * 1024 * 1024);
}

__WEAK int flash_ubi_img(void)
{
    return 0;
}

__WEAK int update_ubi_vol(void)
{
    return 0;
}

__WEAK int target_is_emmc_boot(void)
{
#if _EMMC_BOOT
    return 1;
#else
    return 0;
#endif
}

__WEAK unsigned check_reboot_mode(void)
{
    return 0;
}

__WEAK unsigned check_hard_reboot_mode(void)
{
    return 0;
}

__WEAK void reboot_device(unsigned reboot_reason)
{
}

__WEAK uint32_t is_user_force_reset(void)
{
	return 0;
}

__WEAK int set_download_mode(enum reboot_reason mode)
{
	if(mode == NORMAL_DLOAD || mode == EMERGENCY_DLOAD) {
#if PLATFORM_USE_SCM_DLOAD
		return scm_dload_mode(mode);
#else
		return -1;
#endif
	}

	return 0;
}

__WEAK unsigned target_pause_for_battery_charge(void)
{
    return 0;
}

__WEAK unsigned target_baseband()
{
	return 0;
}

__WEAK void target_serialno(unsigned char *buf)
{
	snprintf((char *) buf, 13, "%s",TARGET(BOARD));
}

__WEAK void target_fastboot_init()
{
}

__WEAK int emmc_recovery_init(void)
{
	return 0;
}

__WEAK bool target_use_signed_kernel(void)
{
#if _SIGNED_KERNEL
	return 1;
#else
	return 0;
#endif
}

__WEAK bool target_is_ssd_enabled(void)
{
#ifdef SSD_ENABLE
	return 1;
#else
	return 0;
#endif
}

__WEAK void target_load_ssd_keystore(void)
{
}

/* Default target does not support continuous splash screen feature. */
__WEAK int target_cont_splash_screen()
{
	return 0;
}

/* Default target specific initialization before using USB */
__WEAK void target_usb_init(void)
{
}

/* Default target specific usb shutdown */
__WEAK void target_usb_stop(void)
{
}

/* Default target specific target uninit */
__WEAK void target_uninit(void)
{
}

__WEAK bool target_display_panel_node(char *pbuf, uint16_t buf_size)
{
	return false;
}

__WEAK void target_display_init(const char *panel_name)
{
}

__WEAK void target_display_shutdown(void)
{
}

__WEAK uint8_t target_panel_auto_detect_enabled()
{
	return 0;
}

__WEAK uint8_t target_is_edp()
{
	return 0;
}

/* default usb controller to be used. */
__WEAK const char * target_usb_controller()
{
	return "ci";
}

/* override for target specific usb phy reset. */
__WEAK void target_usb_phy_reset(void)
{
}

/* determine if target is in warm boot. */
__WEAK bool target_warm_boot(void)
{
	return false;
}

/* Determine the HLOS subtype of the target */
__WEAK uint32_t target_get_hlos_subtype(void)
{
	return 0;
}

/* Initialize crypto parameters */
__WEAK void target_crypto_init_params()
{
}

__WEAK bool target_is_pmi_enabled(void)
{
	return 1;
}

/* Default CFG delay value */
__WEAK uint32_t target_ddr_cfg_val()
{
	return DDR_CONFIG_VAL;
}

/* Target uses system as root */
bool target_uses_system_as_root(void)
{
#if TARGET_USE_SYSTEM_AS_ROOT_IMAGE
	if (target_get_vb_version() >= VB_M)
		return true;
#endif
		return false;
}

/* Check dynamic partition support is enabled for target */
bool target_dynamic_partition_supported(void)
{
#if DYNAMIC_PARTITION_SUPPORT
	return true;
#else
	return false;
#endif
}

/* Default CFG register value */
uint32_t target_ddr_cfg_reg()
{
	uint32_t platform = board_platform_id();
	uint32_t ret = SDCC_HC_REG_DDR_CONFIG;

	switch(platform)
	{
		case MSM8937:
		case MSM8940:
		case APQ8037:
		case MSM8917:
		case MSM8920:
		case MSM8217:
		case MSM8617:
		case APQ8017:
		case MSM8953:
		case APQ8053:
		case SDM450:
		case SDA450:
		case SDM632:
		case SDA632:
		case SDM429:
		case SDM439:
		case SDA429:
		case SDA439:
		case QM215:
		case QCM2150:
		/* SDCC HC DDR CONFIG has shifted by 4 bytes for these platform */
			ret += 4;
			break;
		default:
			break;
	}
	return ret;
}
#if VERIFIED_BOOT || VERIFIED_BOOT_2
int target_get_vb_version()
{
	if (vb_version == INVALID)
	{
		/* Incase of keymaster present,verified boot for M version */
		if (partition_get_index("keymaster") != INVALID_PTN)
			vb_version = VB_M;
		else
		/* Incase keymaster is not present,
		we use keystore for verification. */
			vb_version = VB_L;
	}
	return vb_version;
}
#else
int target_get_vb_version()
{
	return vb_version;
}
#endif

#if VERIFIED_BOOT_LE
int verified_boot_le = 1;
#else
int verified_boot_le = 0;
#endif

int is_vb_le_enabled(void)
{
	uint32_t platform = board_platform_id();

	switch(platform)
	{
		case APQ8053:
		case APQ8009:
			return verified_boot_le;
		default:
			break;
	}
	return 0;
}

#if PON_VIB_SUPPORT
void get_vibration_type(struct qpnp_hap *config)
{
	uint32_t hw_id = board_hardware_id();
	uint32_t platform = board_platform_id();

	config->vib_type = VIB_ERM_TYPE;
	config->hap_rate_cfg1 = QPNP_HAP_RATE_CFG1_1c;
	config->hap_rate_cfg2 = QPNP_HAP_RATE_CFG2_04;
	switch(hw_id){
	case HW_PLATFORM_MTP:
		switch(platform){
		case MSM8952:
			config->vib_type = VIB_ERM_TYPE;
			break;
		case MSM8976:
		case MSM8956:
		case APQ8056:
			config->vib_type = VIB_LRA_TYPE;
			break;
		case MSM8937:
		case MSM8940:
		case APQ8037:
		case MSM8917:
		case MSM8920:
		case MSM8217:
		case MSM8617:
		case APQ8017:
		case MSM8953:
		case APQ8053:
		case SDM450:
		case SDA450:
		case SDM632:
		case SDA632:
		case SDM429:
		case SDM439:
		case SDA429:
		case SDA439:
		case QM215:
		case QCM2150:
		case SDM429W:
			config->vib_type = VIB_LRA_TYPE;
			config->hap_rate_cfg1 = QPNP_HAP_RATE_CFG1_41;
			config->hap_rate_cfg2 = QPNP_HAP_RATE_CFG2_03;
			break;
		default:
			dprintf(CRITICAL,"Unsupported platform id\n");
			break;
		}
		break;
	case HW_PLATFORM_QRD:
		config->vib_type = VIB_ERM_TYPE;
		break;
	default:
		dprintf(CRITICAL,"Unsupported hardware id\n");
		break;
	}
}
#endif

/* Return Build variant */
__WEAK bool target_build_variant_user()
{
#if USER_BUILD_VARIANT
	return true;
#else
	return false;
#endif
}

__WEAK uint32_t target_get_pmic()
{
	return PMIC_IS_UNKNOWN;
}

__WEAK bool is_display_disabled()
{
	return false;
}
/* Check battery if it's exist */
bool target_battery_is_present()
{
	bool batt_is_exist;
	uint8_t value = 0;
	uint32_t pmic;

	pmic = target_get_pmic();

	switch(pmic)
	{
		case PMIC_IS_PM8909:
		case PMIC_IS_PM8916:
		case PMIC_IS_PM8941:
			value = REG_READ(BAT_IF_BAT_PRES_STATUS);
			break;
		case PMIC_IS_PMI8950:
		case PMIC_IS_PMI8994:
		case PMIC_IS_PMI8996:
			if(target_is_pmi_enabled())
			{
				value = REG_READ(PMIC_SLAVE_ID|
						BAT_IF_BAT_PRES_STATUS);
			}
			break;
		case PMIC_IS_PM660:
			value = REG_READ(BAT_IF_INT_RT_STS);
			/* If BAT_TERMINAL_MISSING_RT_STS BIT(5) or BAT_THERM_OR_ID_MISSING_RT_STS BIT(4)
			   are set, battery is not present. */
			if (value & (BIT(5) | BIT(4)))
				return false;
			else
				return true;
			break;
		case PMIC_IS_PMI632:
			value = REG_READ(PMIC_SLAVE_ID|BAT_IF_INT_RT_STS);
			/* If BAT_TERMINAL_MISSING_RT_STS BIT(5) or BAT_THERM_OR_ID_MISSING_RT_STS BIT(4)
			   are set, battery is not present. */
			if (value & (BIT(5) | BIT(4)))
				return false;
			else
				return true;
			break;
		default:
			dprintf(CRITICAL, "ERROR: Couldn't get the pmic type\n");
			break;
	}

	batt_is_exist = value >> 7;

	return batt_is_exist;

}

bool is_target_support_dtbo(void)
{
#if TARGET_DTBO_NOT_SUPPORTED
  return false;
#else
  return true;
#endif
}

#if CHECK_BAT_VOLTAGE
/* Return battery voltage */
uint32_t target_get_battery_voltage()
{
	uint32_t pmic;
	uint32_t vbat = 0;
	uint8_t buff[2];
	uint16_t temp;

	pmic = target_get_pmic();

	switch(pmic)
	{
		case PMIC_IS_PM8909:
		case PMIC_IS_PM8916:
		case PMIC_IS_PM8941:
			vbat = pm8x41_get_batt_voltage(); //uv
			break;
		case PMIC_IS_PMI8950:
		case PMIC_IS_PMI8994:
		case PMIC_IS_PMI8996:
			if(target_is_pmi_enabled())
			{
				if (!pm_fg_usr_get_vbat(1, &vbat)) {
					vbat = vbat*1000; //uv
				} else {
					dprintf(CRITICAL, "ERROR: Get battery voltage failed!!!\n");
				}
			}
			break;
		case PMIC_IS_PM660:
			buff[0] = REG_READ(BATT_INFO_VBATT_LSB);
			buff[1] = REG_READ(BATT_INFO_VBATT_MSB);
			temp = buff[1] << 8 | buff[0];
			/* {MSB,LSB} to decode the voltage level, refer register description. */
			vbat = (((uint64_t)temp)*BATT_VOLTAGE_NUMR/BATT_VOLTAGE_DENR);
			break;
		case PMIC_IS_PMI632:
			buff[0] = REG_READ(ADC_V_DATA_LSB);
			buff[1] = REG_READ(ADC_V_DATA_MSB);
			temp = buff[1] << 8 | buff[0];
			/* {MSB,LSB} to decode the voltage level, refer register description. */
			vbat = (((uint64_t)temp)*QGAUGE_VOLTAGE_NUMR/BATT_VOLTAGE_DENR);
			break;
		default:
			dprintf(CRITICAL, "ERROR: Couldn't get the pmic type\n");
			break;
	}

	return vbat;
}

/* Add safeguards such as refusing to flash if minimum battery levels
 * are not present or be bypass if the device doesn't have a battery
 */
bool target_battery_soc_ok()
{
	if (!target_battery_is_present()) {
		dprintf(INFO, "battery is not present\n");
		return true;
	}

	if (target_get_battery_voltage() >= BATTERY_MIN_VOLTAGE)
		return true;

	return false;
}
#endif
