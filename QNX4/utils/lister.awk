function add_point(chip, pin) {
  point = chip "-" pin
  if (chip == find) {
	save_tree = 1
	if (!(point in found))
	  found[point] = 1
  } else if (!(point in points)) {
	points[point] = 1
  }
}
function end_tree() {
  j = 0;

  if (find != "" && save_tree == 1) {
	print tree
	for (i in found) {
	  if (j + length(i) > 75) {
		printf("\n")
		j = 0
	  }
	  printf(" %s", i)
	  j += length(i)+1
	  delete found[i]
	}
  }
  for (i in points) {
    if (find == "" || save_tree == 1) {
	  if (j + length(i) > 75) {
		printf("\n")
		j = 0
	  }
	  printf(" %s", i)
	  j += length(i)+1
	}
	delete points[i]
  }
  if (find == "" || save_tree == 1) printf("\n")
  save_tree = 0
}
/^\.REM TREE/ {
  end_tree()
  if (find != "") {
	tree = $0
	next
  }
}
/^\.EOD/ {
  end_tree()
}
/^\./ { print; next }
/^ *$/ { next }
{ if (NF != 4) print "Help! NF = " NF
  else {
	add_point($1, $2)
	add_point($3, $4)
  }
}
