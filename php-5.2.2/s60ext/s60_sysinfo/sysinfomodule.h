#ifndef PHP_S60_SYSINFO_H
#define PHP_S60_SYSINFO_H

PHP_FUNCTION(s60_sysinfo_os_version);
PHP_FUNCTION(s60_sysinfo_sw_version);
PHP_FUNCTION(s60_sysinfo_sw_platform);
PHP_FUNCTION(s60_sysinfo_device_model);

PHP_FUNCTION(s60_sysinfo_ram_total);
PHP_FUNCTION(s60_sysinfo_ram_free);
PHP_FUNCTION(s60_sysinfo_rom_total);

PHP_FUNCTION(s60_sysinfo_drive_free);
PHP_FUNCTION(s60_sysinfo_display_twips);
PHP_FUNCTION(s60_sysinfo_display_pixels);
PHP_FUNCTION(s60_sysinfo_battery);
PHP_FUNCTION(s60_sysinfo_battery_max);
PHP_FUNCTION(s60_sysinfo_signal);
PHP_FUNCTION(s60_sysinfo_signal_max);
PHP_FUNCTION(s60_sysinfo_imei);
PHP_FUNCTION(s60_sysinfo_in_emulator);
PHP_FUNCTION(s60_sysinfo_current_profile);
PHP_FUNCTION(s60_message_query);

extern zend_module_entry s60_sysinfo_module_entry;
#define phpext_s60_sysinfo_ptr &s60_sysinfo_module_entry

#endif
