extern void draw_line(int,int,int,int,int,int,int);

#define DRAWBOX(z,z0,q0,z1,q1,q2,q3) \
draw_line(z,z0,q0,z0,q1,q2,q3); \
draw_line(z,z0,q1,z1,q1,q2,q3); \
draw_line(z,z1,q1,z1,q0,q2,q3); \
draw_line(z,z1,q0,z0,q0,q2,q3)
