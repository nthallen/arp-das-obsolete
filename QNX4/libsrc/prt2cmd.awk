function end_group() {
  if (ingroup == 1) print "\t;"
  ingroup = 0
}
/^[a-zA-Z]/ {
  end_group()
  match($0, "  ")
  print "&" substr($0, 1, RSTART)
  ingroup = 1
  next
}
/^ *> / {
  sub("^ *>", "\t:")
  sub(" *\372", " * { ")
  gsub("[\372\r]","")
  gsub("\\( *", "(")
  gsub(" *\\)", ")")
  gsub(" *, *", ", ")
  print $0 " }"
}
END { end_group() }
