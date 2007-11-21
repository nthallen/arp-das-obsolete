#include <stdio.h>
/*#include <alloc.h>*/
#include <stdlib.h>
#include <conio.h>
#include <dir.h>
#include <math.h>
#include <time.h>

#include "navrec.h"
#include "ncom.h"

FILE * arnc_han,*asci_han;
struct STORAGE_DATA *s;
struct NAV_DATA 	*n;

int port;


void Arrange_Buffers();
void Transmit_Data();
void Menu();

void Allocate_Memory()
{
	int i;
	if((s = (struct STORAGE_DATA *) calloc(1,sizeof(struct STORAGE_DATA))) == NULL)
	 printf("Not Possible\n");
	if((n = (struct NAV_DATA *) calloc(1,sizeof(struct NAV_DATA))) == NULL)
	 printf("Not Possible\n");

	n->lat 				= (char *) calloc(12,sizeof(char));
	n->lon 				= (char *) calloc(20,sizeof(char));
	n->gnd_speed 		= (char *) calloc(10,sizeof(char));
	n->tru_trk_angle 	= (char *) calloc(10,sizeof(char));
	n->tru_heading 		= (char *) calloc(10,sizeof(char));
	n->wind_speed 		= (char *) calloc(10,sizeof(char));
	n->wind_dirctn 		= (char *) calloc(10,sizeof(char));
	n->pitch 			= (char *) calloc(10,sizeof(char));
	n->roll				= (char *) calloc(10,sizeof(char));
	n->long_accl 		= (char *) calloc(10,sizeof(char));
	n->lat_accl 		= (char *) calloc(10,sizeof(char));
	n->norm_accl 		= (char *) calloc(10,sizeof(char));
	n->trk_angle_rate 	= (char *) calloc(10,sizeof(char));
	n->pitch_atk_rate 	= (char *) calloc(10,sizeof(char));
	n->roll_atk_rate 	= (char *) calloc(10,sizeof(char));
	n->inertial_alt 	= (char *) calloc(10,sizeof(char));
	n->vert_speed 		= (char *) calloc(10,sizeof(char));

	n->gps_lat 			= (char *) calloc(10,sizeof(char));
	n->gps_lon 			= (char *) calloc(10,sizeof(char));
	n->gps_alt 			= (char *) calloc(10,sizeof(char));

	n->static_presr 	= (char *) calloc(10,sizeof(char));
	n->total_presr 		= (char *) calloc(10,sizeof(char));
	n->diff_presr 		= (char *) calloc(10,sizeof(char));
	n->total_temp 		= (char *) calloc(10,sizeof(char));
	n->static_temp 		= (char *) calloc(10,sizeof(char));
	n->baro_alt 		= (char *) calloc(10,sizeof(char));
	n->mach_no 			= (char *) calloc(10,sizeof(char));
	n->tru_air_speed 	= (char *) calloc(10,sizeof(char));
	n->calc_wind_speed 	= (char *) calloc(10,sizeof(char));
	n->calc_wind_dir 	= (char *) calloc(10,sizeof(char));
	n->ephemeris_elevatn= (char *) calloc(10,sizeof(char));
	n->ephemeris_azimuth= (char *) calloc(10,sizeof(char));
	for(i = 0; i< 12;i++)
		n->anlg_chnl[i]= (char *) calloc(10,sizeof(char));

	n->header[0]=0x01;
	n->header[1]=' ';
	n->crlf[0]=10;
	n->crlf[1]=13;

}


void main(int argc, char *argv[])
{
	int i, j=0;
	time_t  t_start, t_end;
/*	char	drv[MAXDRIVE], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];*/
	char	innm[80], otnm[80];
	unsigned int  time_cnvtd=0;



	if (argc < 2) {
		puts("Usage:  nav_sim filename[.DAT]");
		exit(0);
	}

/*	if ( !(fnsplit(argv[1], drv, dir, name, ext) & EXTENSION) )
		strcpy(ext, ".DAT");

	fnmerge(innm, drv, dir, name, ".DAT");
	drv[0] = getdisk() + 'A';
	drv[1] = ':';
	drv[2] = 0;
	getcurdir(0, dir+1);
	dir[0] = '\\';
	fnmerge(otnm, drv, dir, name, ".ASC");
*/
	strcpy(innm,argv[1]);
	if((arnc_han = fopen( innm, "rb")) == NULL) {
						printf("Cant open file %s\n", innm);
						exit(0);
	}
#if 0
	if((asci_han = fopen( otnm, "w+t")) == NULL) {
						printf("Cant open file %s\n", otnm);
						exit(0);
	}

#endif


	Menu();
	Allocate_Memory();
	Initialize_ComPorts();

//	fwrite(&s,sizeof(storage_data),4,arnc_han);

	fseek(arnc_han, SEEK_SET, 0);

	t_start= time(NULL);
	while( (fread(s,sizeof(struct STORAGE_DATA),1,arnc_han) != 0) && !kbhit() ){
	 printf("DATA TRANSMITTED FOR %u SECONDS\r",time_cnvtd++);


	 while(((t_end=time(NULL)) - t_start) < 1);

	 Arrange_Buffers();
	 Transmit_Data();

	 t_start = t_end;
   }
   Restore_ComPorts();
// fclose(asci_han);
   fclose(arnc_han);
   exit(1);
}



void Arrange_Buffers()
{
	int j = 0,i;
	float t;

	sprintf(n->time,"%03d:%02d:%02d:%02d ",s->gps.days,s->gps.hours,s->gps.min,s->gps.sec);


//	printf("%s ",n->time);

	sprintf(n->lat,			"%c%-8.5f ",(s->inu.lat[j] < 0) ? 'S':'N',fabs(s->inu.lat[j]) );
	sprintf(n->lon,			"%c%-9.5f ",(s->inu.lon[j] < 0) ? 'W':'E',fabs(s->inu.lon[j]) );
//	sprintf(n->lon,			"%-9.5f ",s->inu.lon[j]);
//	printf("%s %s\n",n->lat,n->lon);

	sprintf(n->tru_heading, "%-5.2f ",	((t=s->inu.tru_heading[j]) < 0) ?360 + t : t );
	sprintf(n->pitch,		"%+-8.4f ",	s->inu.pitch[j]);
	sprintf(n->roll,		"%+-8.4f ",	s->inu.roll[j]);
	sprintf(n->gnd_speed,	"%-6.2f ",	s->inu.gnd_speed);
	sprintf(n->tru_trk_angle,"%-6.2f ",	((t=s->inu.tru_trk_angle) < 0) ? 360 + t : t );
	sprintf(n->wind_speed,	"%-4.1f ",	s->inu.wind_speed);
	sprintf(n->wind_dirctn,	"%-5.1f ",	((t=s->inu.wind_dirctn) < 0) ? 360 + t : t);
	sprintf(n->long_accl,	"%+-6.3f ",	s->inu.body_lon_accl);
	sprintf(n->lat_accl,  	"%+-6.3f ",	s->inu.body_lat_accl);
	sprintf(n->norm_accl,	"%+-6.3f ",	s->inu.body_norm_accl);
	sprintf(n->trk_angle_rate,"%+-5.1f ",s->inu.trk_angle_rate);
	sprintf(n->pitch_atk_rate,"%+-5.1f ",s->inu.pitch_atk_rate);
	sprintf(n->roll_atk_rate ,"%+-5.1f ",s->inu.roll_atk_rate);
	sprintf(n->vert_speed,	"%+-6.2f ",	s->inu.inertial_vrtcl_speed);


	sprintf(n->gps_alt,		"%-7.1f ",	s->gps.alt);
	sprintf(n->gps_lat,		"%c%-8.5f ",(s->gps.lat < 0) ? 'S':'N',fabs(s->gps.lat) );
	sprintf(n->gps_lon,		"%c%-9.5f ",(s->gps.lon < 0) ? 'W':'E',fabs(s->gps.lon) );

	sprintf(n->static_presr,"%-8.3f ",s->analog.static_presr * 64.946498);
	sprintf(n->total_presr, "%-8.3f ",s->analog.total_presr * 33.87115);
	sprintf(n->diff_presr,	"%-6.3f ",s->analog.diff_presr);
	sprintf(n->total_temp,	"%+-6.2f ",s->analog.total_temp);
	sprintf(n->static_temp,	"%+-6.2f ",s->analog.static_temp);
	sprintf(n->baro_alt,	"%-7.1f ",(s->analog.baro_alt < 0) ? 0 : s->analog.baro_alt);
	sprintf(n->mach_no,		"%-5.3f ",s->analog.mach_no);
	sprintf(n->tru_air_speed,"%-6.2f ",s->analog.tru_air_speed);
	sprintf(n->calc_wind_speed,"%-4.1f ",s->analog.calc_wind_speed);
	sprintf(n->calc_wind_dir,"%-5.1f ",s->analog.calc_wind_dir);
	sprintf(n->ephemeris_elevatn,"%+-6.2f ",s->analog.ephemeris_elevation);
	sprintf(n->ephemeris_azimuth,"%-6.2f ",s->analog.ephemeris_azimuth);

	for(i = 0; i<12; i++)
	sprintf(n->anlg_chnl[i],"%+-6.3f ",0.0);

}


void Transmit_Data()
{
	int i;
	n->header[0]=0x01;
	n->header[1]='G';
	n->header[2]=' ';
	cm_tx_string(port,n->header);
	cm_tx_string(port,n->time);
	cm_tx_string(port,n->lat);
	cm_tx_string(port,n->lon);
	cm_tx_string(port,n->tru_heading);
	cm_tx_string(port,n->pitch);
	cm_tx_string(port,n->roll);
	cm_tx_string(port,n->gnd_speed);
	cm_tx_string(port,n->tru_trk_angle);
	cm_tx_string(port,n->wind_speed);
	cm_tx_string(port,n->wind_dirctn);
	cm_tx_string(port,n->long_accl);
	cm_tx_string(port,n->lat_accl);
	cm_tx_string(port,n->norm_accl);
	cm_tx_string(port,n->trk_angle_rate);
	cm_tx_string(port,n->pitch_atk_rate);
	cm_tx_string(port,n->roll_atk_rate);
	cm_tx_string(port,n->inertial_alt);
	cm_tx_string(port,n->vert_speed);

	cm_tx_string(port,n->gps_alt);
	cm_tx_string(port,n->gps_lat);
	cm_tx_string(port,n->gps_lon);

	cm_tx_string(port,n->static_presr);
	cm_tx_string(port,n->total_presr);
	cm_tx_string(port,n->diff_presr);
	cm_tx_string(port,n->total_temp);
	cm_tx_string(port,n->static_temp);
	cm_tx_string(port,n->baro_alt);
	cm_tx_string(port,n->mach_no);
	cm_tx_string(port,n->tru_air_speed);
	cm_tx_string(port,n->calc_wind_speed);
	cm_tx_string(port,n->calc_wind_dir);
	cm_tx_string(port,n->ephemeris_elevatn);
	cm_tx_string(port,n->ephemeris_azimuth);

	for(i = 0; i<12; i++)
	cm_tx_string(port,n->anlg_chnl[i]);


	cm_tx_string(port,n->crlf);


}




void Menu()
{
	char ch = 'q';

/*	clrscr();*/
/*	gotoxy(1,4);*/
	printf("SIMULATION OF RS232 OUTPUT OF NAVIGATION DATA RECORDER");
/*	gotoxy(1,5);*/
	printf("THE COM PARMETERS ARE SET AT 9600 BAUD, N, 8, 1 AND ARE NOT CONFIGURABLE");
	while(ch != '1' && ch != '2' && ch != 13){
/*		gotoxy(1,6);*/
		printf("TO SELECT COM1 ENTER 1, TO SELECT COM2 ENTER 2 (DEFAULT COM1): ");
		ch=getch();
	}
	port = (ch == '2') ? QCOM2 : QCOM1;

/*	gotoxy(1,10);*/
	printf("HIT ANY KEY KEY TO QUIT");
/*	gotoxy(1,7);*/



}
