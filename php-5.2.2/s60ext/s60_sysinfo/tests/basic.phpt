--TEST--
Basic s60_sysinfo test.
--FILE--
<?php

echo 'os_version(): ';var_dump(s60_sysinfo_os_version());
echo 'sw_version(): ';var_dump(s60_sysinfo_sw_version());
echo 'device_model(): ';var_dump(s60_sysinfo_device_model());

echo 'ram_total(): ';var_dump(s60_sysinfo_ram_total());
echo 'ram_free(): ';var_dump(s60_sysinfo_ram_free());
echo 'rom_total(): ';var_dump(s60_sysinfo_rom_total());

echo 'drive_free()["C:"]: ';$v = s60_sysinfo_drive_free(); var_dump($v['C:']);
echo 'display_twips(): ';var_dump(s60_sysinfo_display_twips());
echo 'display_pixels(): ';var_dump(s60_sysinfo_display_pixels());
echo 'battery(): ';var_dump(s60_sysinfo_battery());
echo 'battery_max(): ';var_dump(s60_sysinfo_battery_max());
echo 'signal(): ';var_dump(s60_sysinfo_signal());
echo 'signal_max(): ';var_dump(s60_sysinfo_signal_max());
echo 'imei(): ';var_dump(s60_sysinfo_imei());
echo 'in_emulator(): ';var_dump(s60_sysinfo_in_emulator());

?>
--EXPECTF--
os_version(): array(3) {
  ["major"]=>
  int(%d)
  ["minor"]=>
  int(%d)
  ["build"]=>
  int(%d)
}
sw_version(): string(%d) "%s"
device_model(): int(%d)
ram_total(): int(%d)
ram_free(): int(%d)
rom_total(): int(%d)
drive_free()["C:"]: int(%d)
display_twips(): array(2) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
}
display_pixels(): array(2) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
}
battery(): int(%d)
battery_max(): int(%d)
signal(): int(%d)
signal_max(): int(%d)
imei(): string(%d) "%d"
in_emulator(): bool(%s)