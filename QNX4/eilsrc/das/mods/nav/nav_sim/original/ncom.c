#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include <string.h>
#include "ncom.h"

int volatile count = 0,sem=0,ca;
int		cm_irq = -1;
void interrupt	(*cm_oldhndlr[16])() = {
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL };

unsigned int 	cm_port_addr[4] = {CM_COM1, CM_COM2, CM_COM3, CM_COM4};
unsigned int	cm_pic_irq[4] = {4, 3, 4, 3};
struct COMCTL	*cm_com_ctl[4] = {NULL, NULL, NULL, NULL};

int				cm_setup_pic(void);
int				cm_install_irq(int irq);
int				cm_restore_irq(int irq);
void interrupt	cm_handler(void);
void			cm_do_com_int(unsigned int port);
int				cm_init_port(int port, unsigned int r_sz, unsigned int t_sz);
void			cm_lower_dtr(unsigned int port);
void			cm_raise_dtr(unsigned int port);
void			cm_flush_tx(unsigned int port);
void			cm_flush_rx(unsigned int port);
void			cm_tx_string(unsigned int port, char *st);
void			cm_tx(unsigned int port, char ch);
int				cm_tx_empty(unsigned int port);
int				cm_tx_ready(unsigned int port);
int				cm_set_speed(unsigned int port, int speed);
int				cm_set_parity(unsigned int port, enum com_parity parity,
															int stop_bits);
int				cm_rx_empty(unsigned int port);
int				cm_rx_string(unsigned int port, char *in_str,
											unsigned int max_len, char term);
char			cm_rx(unsigned int port);
void			cm_flush_tx(unsigned int port);
void			cm_flush_rx(unsigned int port);
int				cm_carrier(unsigned int port);

void 			Cm_enable_lpbk(unsigned int port);
void 			Cm_disable_lpbk(int port );
char nav_frame[]="Hello this is a test\x03";




void Initialize_ComPorts()
{
	cm_install_irq(3);
	cm_install_irq(4);
	cm_init_port(QCOM2, 256, 256);
	cm_init_port(QCOM1, 256, 256);

	cm_set_parity(QCOM2, cm_none, 1);
	cm_set_speed(QCOM2, 9600);
	cm_raise_dtr(QCOM2);
	cm_set_parity(QCOM1, cm_none, 1);
	cm_set_speed(QCOM1, 9600);
	cm_raise_dtr(QCOM1);

}


void Restore_ComPorts()
{
	cm_restore_irq(3);
	cm_restore_irq(4);
}



void test_main()
{
	int		val;
	char	ch,ch1;
	char	buf[256];

	clrscr();


	printf("%x\t%x\n",inportb(CM_COM2 + CM_MS),inportb(CM_COM2 + CM_LS));


//	Initialize_ComPorts();
//	while((ch = getch()) != 'q'){
//	while(!kbhit()){
//		cm_tx(QCOM2,'a');

//	}

//	getch();

	Restore_ComPorts();
	return(0);
}



int		cm_install_irq(int irq)
{
	if (irq > 15 || irq < 0)
		return(CM_INV_PARAM);
	if (cm_oldhndlr[irq] != NULL)
		return(CM_INT_USED);

	cm_irq = irq;

	if (irq < 8) {
		cm_oldhndlr[irq] = getvect( (irq + 8) );
		setvect( (irq + 8), cm_handler);
		outportb(0x21, inportb(0x21) & ~(1 << irq) );
	}
	else {
		cm_oldhndlr[irq] = getvect( (irq + 0x68) );
				setvect( (irq + 0x68), cm_handler);

		outportb(0xa1, inportb(0xa1) &  ~(1 << (irq - 8)) );
		outportb(0x21, inportb(0x21) & 0xFB );
	}

	enable();
	return(CM_SUCCESS);
}

int		cm_restore_irq(irq)
{
	if (irq > 15 || irq < 0)
		return(CM_INV_PARAM);
	if (cm_oldhndlr[irq] == NULL)
		return(CM_INT_NOT_SET_UP);

//	outportb(CM_PIC + CM_W1, 0xFF);
	cm_irq = -1;

	if (irq < 8) {
		setvect( (irq + 8), cm_oldhndlr[irq]);
		cm_oldhndlr[irq] = NULL;
		outportb(0x21, inportb(0x21) | (1 << irq) );
	}
	else {
		setvect( (irq + 0x68), cm_oldhndlr[irq]);
		cm_oldhndlr[irq] = NULL;
		outportb(0xa1, inportb(0xa1) | (1 << (irq - 8)) );
	}
	return(CM_SUCCESS);
}


int		cm_setup_pic()
{
	outportb(CM_PIC + CM_W0, 0x12);	/*	ICW1: Single 8259, use ICW4		*/
	outportb(CM_PIC + CM_W1, 0);	/*	ICW2: No vector address			*/
//	outportb(CM_PIC + CM_W1, 0x01);	/*	ICW4: 8086/88 mode				*/
//	outportb(CM_PIC + CM_W1, 0xFF);	/*	OCW1: MAsk all PIC interrupts	*/

	return(CM_SUCCESS);
}

int		cm_poll_pic()
{
//	outportb(CM_PIC + CM_W0, 0x0C);		/* Issue poll command to pic 0cw3		*/
	return( inportb(CM_PIC + CM_W0) );
}

void interrupt	cm_handler(void)
{
	int		irq;
	outportb(CM_PIC + CM_W0 , 0x0b);		/* Issue command to pic 0cw3 to read ISR		*/
	irq = inportb(CM_PIC + CM_W0);          // Read the ISR register
	count = irq & 0x18;

		switch (irq & 0x18) {
			case 8:
				cm_do_com_int(QCOM2);
//				cm_irq = 3;
				break;
			case 16:
				cm_do_com_int(QCOM1);
//				cm_irq = 4;
				break;
			default:
				break;
		}

	if (cm_irq > 7)
		outportb(0xa0, 0x20);
	outportb(0x20, 0x20);
	enable();
}

/*
Interrupt driver.  The UART is programmed to cause an interrupt whenever
a character has been received or when the UART is ready to transmit another
character.
*/
void	cm_do_com_int(unsigned int port)
{
	struct COMCTL	*ctl;
	char			ch;
	int				iir;

/*	Get address of current control structure		*/
	ctl = cm_com_ctl[port];
	port = cm_port_addr[port];

/*	While bit 0 of the interrupt identification register is 0, there is an	*/
/*	interrupt to process													*/

	while ( ((iir = inportb(port + CM_IIR)) & 1) == 0) {
		switch ((iir >> 1) & 3) {

			/*	iir = 100b: Received data available.  Get the character, 	*/
			/*	and if the buffer isn't full, then save it.  If the buffer	*/
			/*	is full, then ignore it.									*/

			case 2:
				ch = inportb(port + CM_RXD);
				if (ctl->rx_chars <= ctl->rx_queue_size) {
					ctl->rx_queue[ctl->rx_in++] = ch;
					if (ctl->rx_in >= ctl->rx_queue_size)
						ctl->rx_in = 0;
					ctl->rx_chars++;
				}
				break;

			/*	iir = 010b: Transmit register empty.  If the transmit 		*/
			/*	buffer is empty, then disable the transmitter to prevent 	*/
			/*	any more transmit interrupts.  Otherwise, send the 			*/
			/*	character.													*/

			/*	The test of the line-status-register is to see if the 		*/
			/*	transmit holding register is truly empty.  Some UARTS seem 	*/
			/*	to cause transmit interrupts when the holding register 		*/
			/*	isn't empty, causing transmitted characters to be lost.		*/
			case 1:
				if (ctl->tx_chars <= 0) {
					outportb(port + CM_IER, inportb(port + CM_IER) & 0xfd);
					ctl->tx_running = FALSE;
				}
				else if (inportb(port + CM_LS) & 0x20) {
					outportb(port + CM_TXD, ctl->tx_queue[ctl->tx_out++]);
					if (ctl->tx_out >= ctl->tx_queue_size)
						ctl->tx_out = 0;
					ctl->tx_chars--;
				}
				break;

			/*	iir = 001b: Change in modem status.  We don't expect this	*/
			/*	interrupt, but if one ever occurs we need to read the line	*/
			/*	status to reset it and prevent an endless loop.				*/
			case 0:
				inportb(port + CM_MS);
				break;

			/*	iir = 111b: Change in line status.  We don't expect this	*/
			/*	interrupt, but if one ever occurs we need to read the line	*/
			/*	status to reset it and prevent an endless loop.				*/
			case 3:
				inportb(port + CM_LS);
				break;
		}
	}
}

int		cm_init_port(int port, unsigned int rx_size, unsigned int tx_size)
{
	unsigned int	irq;

	if (port > 3)
		return(CM_INV_PARAM);

	cm_com_ctl[port] = (struct COMCTL *)calloc(1, sizeof(struct COMCTL));
	if (cm_com_ctl[port] == NULL)
		return(CM_OUT_OF_MEMORY);

	cm_com_ctl[port]->tx_queue = (char *)calloc(1, tx_size);
	cm_com_ctl[port]->rx_queue = (char *)calloc(1, rx_size);
	if (cm_com_ctl[port]->tx_queue == NULL) {
		free(cm_com_ctl[port]);
		cm_com_ctl[port] = NULL;
		return(CM_OUT_OF_MEMORY);
	}
	if (cm_com_ctl[port]->rx_queue == NULL) {
		free(cm_com_ctl[port]);
		cm_com_ctl[port] = NULL;
		return(CM_OUT_OF_MEMORY);
	}

	cm_com_ctl[port]->tx_queue_size = tx_size;
	cm_com_ctl[port]->rx_queue_size = rx_size;
	port = cm_port_addr[port];
	outportb(port + CM_IER, inportb(port + CM_IER) | 0x0D);
	outportb(port + CM_MC, inportb(port + CM_MC) | 0x08);
	return(CM_SUCCESS);
}


/*	Lower (deactivate) the DTR line.  Causes most modems to hang up.		*/

void	cm_lower_dtr(unsigned int port)
{
	if ( cm_com_ctl[port] != NULL ) {
		port = cm_port_addr[port] + CM_MC;
		disable();
		outportb(port, inportb(port) & 0xfe);
		enable();
	}
}


/*	Raise (activate) the DTR line.											*/

void	cm_raise_dtr(unsigned int port)
{
	if ( cm_com_ctl[port] != NULL ) {
		port = cm_port_addr[port] + CM_MC;
		disable();
		outportb(port, inportb(port) | 1);
		enable();
	}
}

/*	Flush (empty) the receive buffer.										*/

void	cm_flush_rx(unsigned int port)
{
	struct COMCTL	*ctl;

	if ( cm_com_ctl[port] != NULL ) {
		ctl = cm_com_ctl[port];
		disable();
		ctl->rx_chars = 0;
		ctl->rx_in    = 0;
		ctl->rx_out   = 0;
		enable();
	}
}


/*	Flush (empty) transmit buffer.											*/

void	cm_flush_tx(unsigned int port)
{
	struct COMCTL	*ctl;

	if ( cm_com_ctl[port] != NULL ) {
		ctl = cm_com_ctl[port];
		disable();
		ctl->tx_chars = 0;
		ctl->tx_in    = 0;
		ctl->tx_out   = 0;
		enable();
	}
}

/*	Get a character from the receive buffer.  If the buffer is empty, 		*/
/*	return a NULL (#0).														*/

char	cm_rx(unsigned int port)
{
	char	i;
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];

	if ( ctl && ctl->rx_chars) {
		i = ctl->rx_queue[ctl->rx_out++];
		if (ctl->rx_out >= ctl->rx_queue_size)
			ctl->rx_out = 0;
		ctl->rx_chars--;
		return(i);
	}
	else
		return(0);
}


/*	Receive a string of characters.  The received characters are stored in	*/
/*	in_str, to a maximum of max_len characters.  If a terminator, term, is 	*/
/*	found, the input string is terminated with a null and the function		*/
/*	returns.  If max_len characters are received, the string is terminated	*/
/*	with a null.  This function will wait forever for a terminator or 		*/
/*	max_len characters.	 The function returns the number of characters		*/
/*	actually received, not including the terminator.						*/

int	cm_rx_string(unsigned int port, char *in_str,
											unsigned int max_len, char term)
{
	char	ch;
	char	*cp;

	cp = in_str;

	while ( (cp - in_str) < max_len ) {
		ch = cm_rx(port);

		if (ch == '\0') continue;
		if (ch == term) break;

		*cp++ = ch;
	}
	*cp = '\0';
	return(cp - in_str);
}


/*	This function returns True if the receive buffer is empty.				*/

int	cm_rx_empty(unsigned int port)
{
	int		nchr;
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];
	if ( ctl && ctl->rx_chars) {
		disable();
		nchr = ctl->rx_chars;
		enable();
	}

	return( (nchr == 0) || !ctl );
}

/*	Set the parity and stop bits as follows:

		com_none    8 data bits, no parity
		com_even    7 data bits, even parity
		com_odd     7 data bits, odd parity
		com_zero    7 data bits, parity always zero
		com_one     7 data bits, parity always one
		com_O81		8 data bits, odd parity, 1 stop bit
*/

int		cm_set_parity(unsigned int port, enum com_parity parity,
															int stop_bits)
{
	int	lcr;

	if (cm_com_ctl[port] == NULL)
		return(CM_PORT_NOT_SET_UP);
	port = cm_port_addr[port] + CM_LC;

	switch (parity) {
		case cm_none:
			lcr = 0x03;
			break;
		case cm_even:
			lcr = 0x1a;
			break;
		case cm_odd :
			lcr = 0x0a;
			break;
		case cm_zero:
			lcr = 0x3a;
			break;
		case cm_one :
			lcr = 0x2a;
			break;
		case cm_O81 :
			lcr = 0x0b;
			break;
	}
	if (stop_bits == 2)
		lcr |= 4;
	disable();
	outportb(port, (inportb(port) & 0x40) | lcr);
	enable();
	return(CM_SUCCESS);
}

/*	Set the baud rate.  Accepts any speed between 2 and 65535.  However,	*/
/*	I am not sure that extremely high speeds (those above 19200) will		*/
/*	always work, since the baud rate divisor will be six or less, where a	*/
/*	difference of one can represent a difference in baud rate of			*/
/*	3840 bits per second or more.											*/

int		cm_set_speed(unsigned int port, int speed)
{
	unsigned int	divisor;

	if ( cm_com_ctl[port] != NULL ) {
		port = cm_port_addr[port];
		if (speed < 2) speed = 2;
		divisor = (unsigned int)(115200L / (long)speed);
		disable();
		outportb(port + CM_LC, inportb(port + CM_LC) | 0x80);
		outport(port + CM_DLL, divisor);
		outportb(port + CM_LC, inportb(port + CM_LC) & 0x7f);
		enable();
		return(CM_SUCCESS);
	}
	return(CM_PORT_NOT_SET_UP);
}

/*	This function returns TRUE if com_tx can accept a character.			*/

int	cm_tx_ready(unsigned int port)
{
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];
	return( (ctl->tx_chars < ctl->tx_queue_size) && ctl);
}


/*	This function returns True if the transmit buffer is empty.				*/

int	cm_tx_empty(unsigned int port)
{
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];
	return( (ctl->tx_chars == 0) || !ctl);
}

/*	Send a character.  Waits until the transmit buffer isn't full, then		*/
/*	puts the character into it.  											*/

void	cm_tx(unsigned int port, char ch)
{
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];
	if (ctl) {
		while ( !cm_tx_ready(port) ) ;
		ctl->tx_queue[ctl->tx_in++] = ch;
		if (ctl->tx_in >= ctl->tx_queue_size)
			ctl->tx_in = 0;
		disable();
		ctl->tx_chars++;
		port = cm_port_addr[port];
		outportb(port + CM_IER, inportb(port + CM_IER) | 2);
		if (!ctl->tx_running && ctl->tx_chars) {
			ch = ctl->tx_queue[ctl->tx_out++];
			if (ctl->tx_out >= ctl->tx_queue_size)
				ctl->tx_out = 0;
			ctl->tx_chars--;
			ctl->tx_running = TRUE;
			outportb(port + CM_TXD, ch);
		}
		enable();
	}
}


/*	Send a whole string														*/

void	cm_tx_string(unsigned int port, char	*st)
{
	while (*st != '\0'){
//		putch(*st);
		cm_tx(port, *st++);
	}
}



/*	This function returns TRUE if a carrier is present.						*/

int	cm_carrier(unsigned int port)
{
	struct COMCTL	*ctl;

	ctl = cm_com_ctl[port];
	port = cm_port_addr[port];
	return( ctl && (inportb(port + CM_MS) & 0x80) );
}

/* This fn sets the UART to enable its internal loop back*/
void Cm_enable_lpbk(unsigned int port)
{
	//struct COMCTL	*ctl;

	//	ctl = cm_com_ctl[port];
		port = cm_port_addr[port];
		//disable();
		//printf("%x\n", inportb(port + CM_MC));
		outportb(port + CM_MC,inportb(port + CM_MC) | 0x10);
		//printf("%x\n", inportb(port + CM_MC));
		//enable();
}

/*This fn sets the UART to diable its internal lpbk*/
void Cm_disable_lpbk(int port )
{
		port = cm_port_addr[port];
		disable();
		outportb(port + CM_MC,inportb(port + CM_MC) & 0xEF);
		enable();
}
