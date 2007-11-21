/*	Defines for ZT8902 mother board com ports*/
#ifndef TRUE
	#define	TRUE	(1 == 1)
#endif
#ifndef FALSE
	#define	FALSE	(1 == 0)
#endif

#define	CM_SUCCESS			0
#define	CM_INV_PARAM		1
#define	CM_INT_USED			2
#define	CM_INT_NOT_SET_UP	3
#define	CM_OUT_OF_MEMORY	4
#define CM_PORT_NOT_SET_UP	5

/*	Board addresses	*/
#define	CM_COM1		0x3F8
#define	CM_COM2		0x2F8
#define	CM_COM3		0x3E8
#define	CM_COM4		0x2E8
#define	CM_PIC		0x20


/*	UART Register offsets	*/
#define	CM_RXD		0x00
#define	CM_TXD		0x00
#define	CM_DLL		0x00
#define	CM_IER		0x01
#define	CM_DLM		0x01
#define	CM_IIR		0x02
#define	CM_LC		0x03
#define	CM_MC		0x04
#define	CM_LS		0x05
#define	CM_MS		0x06

/*	Programmable interrupt controller offsets		*/
#define	CM_W0		0x00
#define	CM_W1		0x01

/*	Port values	for control routines			*/
#define	QCOM1		0
#define	QCOM2		1
#define	QCOM3		2
#define	QCOM4		3

enum	com_parity	{cm_none, cm_even, cm_odd, cm_zero, cm_one, cm_O81};

struct COMCTL {
	unsigned int		rx_queue_size;
	unsigned int volatile	rx_chars;
	unsigned int volatile	rx_in;
	unsigned int			rx_out;
	char					*rx_queue;
	unsigned int			tx_queue_size;
	unsigned int volatile	tx_chars;
	unsigned int			tx_in;
	unsigned int volatile	tx_out;
	char					*tx_queue;
	int volatile			tx_running;
};



#define inportb(X) inp(X)
#define outportb(X,Y) outp(X,Y)
#define outport(X,Y) outpw(X,Y)
#define enable() _enable()
#define disable() _disable()



