# chips[name] = 1
# nextchip[name] = name'
# nextchip[""] = firstname
# pins[name,pin] = 1
# nextpin[name,pin] = pin'
# nextpin[name,""] = pin
function add_point(chip, pin) {
  pchip = ""
  for (;;) {
	nchip = nextchip[pchip]
	if (nchip == "" || chip < nchip) {
	  nextchip[pchip] = chip
	  nextchip[chip] = nchip
	  break
	} else if (chip == nchip) break
	else pchip = nchip
  }
  ppin = ""
  for (;;) {
	npin = nextpin[chip,ppin]
	if (npin == "" || pin < npin) {
	  nextpin[chip,ppin] = pin
	  nextpin[chip,pin] = npin
	  break
	} else if (pin == npin) break
	else ppin = npin
  }
}
function end_tree() {
  j = 0;

  for (pchip = ""; ;) {
	chip = nextchip[pchip]
	delete nextchip[pchip]
	if (chip == "") break
	pchip = chip

	for (ppin = ""; ;) {
	  pin = nextpin[chip,ppin]
	  delete nextpin[chip,ppin]
	  if (pin == "") break
	  ppin = pin
	  
	  pintext = " " chip "-" pin
	  if (j + length(pintext) > 75) {
		printf("\n")
		j = 0
	  }
	  printf pintext
	  j += length(pintext)
	}
  }
  printf "\n"
}
/^\.REM TREE/ {
  end_tree()
}
/^\.EOD/ {
  end_tree()
}
/^\./ { print; next }
/^ *$/ { next }
{
  for (i = 1; i < NF; i+=2) {
	if (i == NF) print "Help, NF is odd"
	else add_point($i, $(i+1))
  }
}
