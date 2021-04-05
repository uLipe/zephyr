:orphan:

.. _zephyr_2.6:

Zephyr 2.6.0 (Working Draft)
############################

We are pleased to announce the release of Zephyr RTOS version 2.6.0.

Major enhancements with this release include:

The following sections provide detailed lists of changes by component.

Security Vulnerability Related
******************************

The following CVEs are addressed by this release:

More detailed information can be found in:
https://docs.zephyrproject.org/latest/security/vulnerabilities.html

Known issues
************

You can check all currently known issues by listing them using the GitHub
interface and listing all issues with the `bug label
<https://github.com/zephyrproject-rtos/zephyr/issues?q=is%3Aissue+is%3Aopen+label%3Abug>`_.

API Changes
***********

* The :c:func:`wait_for_usb_dfu` function now accepts a ``k_timeout_t`` argument instead of
  using the ``CONFIG_USB_DFU_WAIT_DELAY_MS`` macro.

Deprecated in this release

* :c:macro:`DT_CLOCKS_LABEL_BY_IDX`, :c:macro:`DT_CLOCKS_LABEL_BY_NAME`,
  :c:macro:`DT_CLOCKS_LABEL`, :c:macro:`DT_INST_CLOCKS_LABEL_BY_IDX`,
  :c:macro:`DT_INST_CLOCKS_LABEL_BY_NAME`, and
  :c:macro:`DT_INST_CLOCKS_LABEL` was deprecated in favor of utilizing
  :c:macro:`DT_CLOCKS_CTLR` and variants.

* :c:macro:`DT_PWMS_LABEL_BY_IDX`, :c:macro:`DT_PWMS_LABEL_BY_NAME`,
  :c:macro:`DT_PWMS_LABEL`, :c:macro:`DT_INST_PWMS_LABEL_BY_IDX`,
  :c:macro:`DT_INST_PWMS_LABEL_BY_NAME`, and
  :c:macro:`DT_INST_PWMS_LABEL` was deprecated in favor of utilizing
  :c:macro:`DT_PWMS_CTLR` and variants.

* :c:macro:`DT_IO_CHANNELS_LABEL_BY_IDX`,
  :c:macro:`DT_IO_CHANNELS_LABEL_BY_NAME`,
  :c:macro:`DT_IO_CHANNELS_LABEL`,
  :c:macro:`DT_INST_IO_CHANNELS_LABEL_BY_IDX`,
  :c:macro:`DT_INST_IO_CHANNELS_LABEL_BY_NAME`, and
  :c:macro:`DT_INST_IO_CHANNELS_LABEL` were deprecated in favor of utilizing
  :c:macro:`DT_IO_CHANNELS_CTLR` and variants.

* :c:macro:`DT_DMAS_LABEL_BY_IDX`,
  :c:macro:`DT_DMAS_LABEL_BY_NAME`,
  :c:macro:`DT_INST_DMAS_LABEL_BY_IDX`, and
  :c:macro:`DT_INST_DMAS_LABEL_BY_NAME` were deprecated in favor of utilizing
  :c:macro:`DT_DMAS_CTLR` and variants.

* USB HID specific macros in ``<include/usb/class/usb_hid.h>`` are deprecated
  in favor of new common HID macros defined in ``<include/usb/class/hid.h>``.

* USB HID Kconfig option USB_HID_PROTOCOL_CODE is deprecated.
  USB_HID_PROTOCOL_CODE does not allow to set boot protocol code for specific
  HID device. USB HID API function usb_hid_set_proto_code() can be used instead.

* The ``CONFIG_OPENOCD_SUPPORT`` Kconfig option has been deprecated in favor
  of ``CONFIG_DEBUG_THREAD_INFO``.

* Disk drivers (``disk_access_*.c``) are moved to ``drivers/disk`` and renamed
  according to their function. Driver's Kconfig options are revised and renamed.
  SDMMC host controller drivers are selected when the corresponding node
  in devicetree is enabled. Following application relevant Kconfig options
  are renamed: ``CONFIG_DISK_ACCESS_RAM`` -> `CONFIG_DISK_DRIVER_RAM`,
  ``CONFIG_DISK_ACCESS_FLASH`` -> `CONFIG_DISK_DRIVER_FLASH`,
  ``CONFIG_DISK_ACCESS_SDHC`` -> `CONFIG_DISK_DRIVER_SDMMC`.
  Disk API header ``<include/disk/disk_access.h>`` is deprecated in favor of
  ``<include/storage/disk_access.h>``.

* :c:func:`flash_write_protection_set()`.

==========================

Removed APIs in this release

* Removed support for the old zephyr integer typedefs (u8_t, u16_t, etc...).

* Removed support for k_mem_domain_destroy and k_mem_domain_remove_thread

* Removed support for counter_read and counter_get_max_relative_alarm

* Removed support for device_list_get

============================

Stable API changes in this release
==================================

Kernel
******

Architectures
*************

* ARC

* ARM

  * AARCH32

    * Added support for null pointer dereferencing detection in Cortex-M.

    * Added initial support for Arm v8.1-m and Cortex-M55

  * AARCH64

* POSIX

* RISC-V

* x86

Boards & SoC Support
********************

* Added support for these SoC series:

* Removed support for these SoC series:

   * ARM Musca-A

* Made these changes in other SoC series:

* Changes for ARC boards:

* Added support for these ARM boards:

   * MPS3-AN547

* Removed support for these ARM boards:

   * ARM V2M Musca-A
   * Nordic nRF5340 PDK

* Made these changes in other boards:

* Added support for these following shields:

Drivers and Sensors
*******************

* ADC

* Audio

* Bluetooth

  * The Kconfig option ``CONFIG_BT_CTLR_TO_HOST_UART_DEV_NAME`` was removed.
    Use the :ref:`zephyr,bt-c2h-uart chosen node <devicetree-chosen-nodes>`
    directly instead.

* CAN

* Clock Control

* Console

* Counter

* Crypto

* DAC

* Debug

* Display

* DMA

* EEPROM

* Entropy

* ESPI

* Ethernet

* Flash

  * flash_write_protection_set() has been deprecated and will be removed in
    Zephyr 2.8. Responsibility for write/erase protection management has been
    moved to the driver-specific implementation of the flash_write() and
    flash_erase() API calls. All in-tree flash drivers have been updated,
    and the protect implementation removed from their API tables.
    During the deprecation period user code invoking
    flash_write_protection_set() will have no effect, but the flash_write() and
    flash_erase() driver shims will wrap their calls with calls to the protect
    implementation if it is present in the API table.
    Out-of-tree drivers must be updated before the wrapping in the shims is
    removed when the deprecation period ends.

* GPIO

* Hardware Info

* I2C

* I2S

* IEEE 802.15.4

* Interrupt Controller

* IPM

* Keyboard Scan

* LED

* LED Strip

* LoRa

* Modem

* PECI

* Pinmux

* PS/2

* PWM

* Sensor

* Serial

* SPI

* Timer

* USB

* Video

* Watchdog

* WiFi

Networking
**********

Bluetooth
*********

* Host

* Mesh

* BLE split software Controller

* HCI Driver

Build and Infrastructure
************************

* Improved support for additional toolchains:

* Devicetree

  - :c:macro:`DT_COMPAT_GET_ANY_STATUS_OKAY`: new macro

Libraries / Subsystems
**********************

* Disk

* Management

  * MCUmgr

  * updatehub

* Settings

* Random

* POSIX subsystem

* Power management

* Logging

* LVGL

* Shell

* Storage

* Tracing

* Debug

HALs
****

* HALs are now moved out of the main tree as external modules and reside in
  their own standalone repositories.

Documentation
*************

Tests and Samples
*****************

* Twister's ``dt_compat_enabled_with_alias()`` test case filter was deprecated
  in favor of a new ``dt_enabled_alias_with_parent_compat()`` filter. The old
  filter is still supported, but it may be removed in a future release.

  To update, replace uses like this:

  .. code-block:: yaml

     filter: dt_compat_enabled_with_alias("gpio-leds", "led0")

  with:

  .. code-block:: yaml

     filter: dt_enabled_alias_with_parent_compat("led0", "gpio-leds")

Issue Related Items
*******************

These GitHub issues were addressed since the previous 2.5.0 tagged
release:
