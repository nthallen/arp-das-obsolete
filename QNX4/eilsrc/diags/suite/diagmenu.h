/* include the diagnostics you want in this file */

/* list of implemented tests to choose from */
/*
-------------------------------------------------------------------------
  "SUBBUS Low Byte:", "sbl",  subbus_low,
  "SUBBUS High Byte:", "sbh",  subbus_high,
  "SUBBUS Word:", "sbw",  subbus_word,
  "Non-Volatile RAM:", "pch",  nv_ram_test,
  "RAM Pattern Test:", "pat",  pattern_test,
  "CMDENBL Test:", "cmd",  cmdenbl_test,
  "Reboot Circuitry:", "rbt",  reboot_test,
  "Input Port:", "inp",  input_port,
  "NMI Test:", "nmi",  nmi_test
  "Addr Debug:", "adb", addr_debug,
  "Card AtoD0:", "ad0", AtoD0,
  "Card AtoD1:", "ad0", AtoD1,
  "Card AtoD2:", "ad0", AtoD2,
  "Card AtoD3:", "ad0", AtoD3,
  "Card AtoD4:", "ad0", AtoD4,
  "Card AtoD5:", "ad0", AtoD5,
  "Card AtoD6:", "ad0", AtoD6,
  "DtoA Converter:", "d2a", DtoAtest
-----------------------------------------------------------------------
*/

/* menu name, 3 code mnemonic, name of diagnostic routine */
#ifdef card
  "Addr Debug:", "adb", addr_debug,
  "Card AtoD0:", "ad0", AtoD0,
  "Card AtoD1:", "ad0", AtoD1,
  "Card AtoD2:", "ad0", AtoD2,
  "Card AtoD3:", "ad0", AtoD3,
  "Card AtoD4:", "ad0", AtoD4,
  "Card AtoD5:", "ad0", AtoD5,
  "Card AtoD6:", "ad0", AtoD6,
  "Card H2O:",   "h2o", H2O,
  "DtoA Converter:", "d2a", DtoAtest
#endif

#ifdef syscon
  "SUBBUS Low Byte:", "sbl",  subbus_low,
  "SUBBUS High Byte:", "sbh",  subbus_high,
  "SUBBUS Word:", "sbw",  subbus_word,
  "Non-Volatile RAM:", "pch",  nv_ram_test,
  "RAM Pattern Test:", "pat",  pattern_test,
  "CMDENBL Test:", "cmd",  cmdenbl_test,
  "Reboot Circuitry:", "rbt",  reboot_test,
  "Input Port:", "inp",  input_port /* , */
/*  "NMI Test:", "nmi",  nmi_test */
#endif


