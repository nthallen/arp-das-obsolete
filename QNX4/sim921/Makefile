OBJ = sim921.o sim921oui.o
sim921 : $(OBJ)
	$(LINK.c) $(LIBFLAGS) -o $@ $(LDFLAGS) $(OBJ)
	usemsg sim921 sim921oui.c
sim921oui.c : sim921.oui
	oui -o sim921oui.c sim921.oui
# sim921oui.o : sim921oui.c sim921.h
# sim921.o : sim921.c sim921.h
$(OBJ) : sim921.h
clean :
	rm $(OBJ) sim921oui.c sim921
