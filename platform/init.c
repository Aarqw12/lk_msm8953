/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Copyright (c) 2014-2016,2019 The Linux Foundation. All rights reserved.
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
#include <platform.h>
#include <boot_stats.h>
#include <platform/iomap.h>
#include <image_verify.h>
#include <target.h>
#include <boot_device.h>
#include <mmc_wrapper.h>

/*
 * default implementations of these routines, if the platform code
 * chooses not to implement.
 */
__WEAK int platform_use_identity_mmu_mappings(void)
{
	return 1;
}

__WEAK void platform_init_mmu_mappings(void)
{
}

__WEAK addr_t platform_get_virt_to_phys_mapping(addr_t virt_addr)
{
	ASSERT(platform_use_identity_mmu_mappings());

	return virt_addr;
}

__WEAK addr_t platform_get_phys_to_virt_mapping(addr_t phys_addr)
{
	ASSERT(platform_use_identity_mmu_mappings());

	return phys_addr;
}

__WEAK void platform_early_init(void)
{
}

__WEAK void platform_init(void)
{
}

__WEAK void display_init(void)
{
}

__WEAK void display_shutdown(void)
{
}

__WEAK void platform_config_interleaved_mode_gpios(void)
{
}

__WEAK void uart_clock_init(void)
{
}

__WEAK void platform_uninit(void)
{
}

__WEAK int image_verify(unsigned char * image_ptr,
			unsigned char * signature_ptr,
			unsigned int image_size,
			unsigned hash_type)
{
	return 0;
}

__WEAK void ce_clock_init(void)
{
}

__WEAK addr_t get_bs_info_addr()
{
	return 0;
}

__WEAK uint32_t platform_get_sclk_count(void)
{
	return 0;
}

__WEAK void clock_config_cdc(uint32_t slot)
{

}

__WEAK int get_target_boot_params(const char *cmdline, const char *part,
				  char **buf)
{
	return -1;
}

__WEAK uint32_t platform_get_smem_base_addr()
{
	return (uint32_t)MSM_SHARED_BASE;
}

__WEAK uint32_t platform_boot_dev_isemmc()
{
        return 1;
}

__WEAK uint32_t platform_boot_dev_is_nand()
{
        return 0;
}

__WEAK uint32_t platform_get_boot_dev()
{
        return 0;
}

__WEAK int boot_device_mask(int val)
{
	return ((val & 0x3E) >> 1);
}

__WEAK uint32_t platform_detect_panel()
{
	return 0;
}

__WEAK bool use_hsonly_mode()
{
	return 0;
}

__WEAK uint32_t check_alarm_boot()
{
	return 0;
}
__WEAK uint32_t platform_get_max_periph()
{
	return 128;
}

/* This function definition should not be used if VERIFIED_BOOT
is enabled. And this is expected to be called only for MDM Targets */
__WEAK uint32_t read_der_message_length(unsigned char* input, unsigned sz)
{
	return SIGNATURE_SIZE;
}

/* Check if glink is supported or not */
__WEAK bool platform_is_glink_enabled()
{
#if GLINK_SUPPORT
	return 1;
#else
	return 0;
#endif
}

/* function to update boot device base, for emmc targets. */
__WEAK void platform_boot_dev_cmdline(char *buf)
{
	struct mmc_device *dev = NULL;
	uint32_t boot_dev_str_sz = 0;

	if (!buf) {
		dprintf(CRITICAL,"ERROR: Invalid args- Failed to populate boot device");
		return;
	}

	dev = (struct mmc_device *)target_mmc_device();
	boot_dev_str_sz = ((sizeof(dev->host.base))*2) + 7;
	if (boot_dev_str_sz > sizeof(char) * BOOT_DEV_MAX_LEN) {
		dprintf(CRITICAL,"ERROR: Invalid buf sz - Failed to populate boot device");
		return;
	}

	snprintf(buf, boot_dev_str_sz, "%x.sdhci", dev->host.base);
	return;
}
