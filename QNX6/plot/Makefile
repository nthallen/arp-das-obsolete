CPPFLAGS=-I /usr/local/include
CXXFLAGS=-g
LDFLAGS=-lnort -lcpp -lph
CC=gcc

OBJ=axes.o f_matrix.o figure.o line.o plot.o plot_obj.o polyline.o scale.o
plot : $(OBJ)
$(OBJ) : plot.h f_matrix.h
