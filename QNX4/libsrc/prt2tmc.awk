/\372Address/ {
  sub("\372", "") # address
  sub("\372", "") # mnemonic
  sub("\372", "0x") # address: add hex
}
/\372/ {
  gsub("\372", "");
  sub(" *;", ";");
  sub(" *>", ""); # spaces at the beginning of comment
  sub(" *\\*/", " */"); # spaces at the end of comment
  print; next
}
