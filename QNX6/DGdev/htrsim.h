typedef struct {
  double I;     /* Amps through heater */
  double R;     /* Heater's Electrical Resistance in Ohms */
  double Rt;    /* Heater's Thermal Resistivity C/W */
  double Ct;    /* Heater's Thermal Mass C/J */
  double Tamb;  /* Ambient Temperature C */
  double Gp;    /* Proportional Gain 1/C */
  double Gi;    /* Integral Gain 1/(C*Sec) */
  double Gd;    /* Differential Gain Sec/C */
  double Simax; /* Maximum Integral Term (before gain) C*Sec */
  double Tset;  /* Temperature Set Point */
} Htr_params;

Htr_params HtrData = {
  1.0, 1.0, 1.0, 1.0, 20.0,
  0., 0., 0., 0., 20.0
};
