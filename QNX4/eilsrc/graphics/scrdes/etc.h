/* key definitions, etc */

#ifdef __QNX__

#define DIRSLASH '/'
#define BACKSPACE 127
/* the alt-alphabets are function codes for QNX 4 */
#define LINE 492       /* Alt l */
#define TEXT 500       /* ALt t */
#define ERASE 485      /* Alt e */
#define BUFFER 482     /* Alt b */
#define FILL 486       /* Alt f */
#define FIELD 489      /* Alt i */
#define SCREEN 499     /* alt s */
#else

#define DIRSLASH '\\'
#define BACKSPACE 8
#define LINE 294       /* Alt l */
#define TEXT 276       /* Alt t */
#define ERASE 274      /* Alt e */
#define BUFFER 304     /* Alt b */
#define FILL 289       /* Alt f */
#define FIELD 279      /* Alt i */
#define SCREEN 287     /* alt s */
#endif

#define CR 10
#define ESC 27
#define INS KEY_INS
#define DEL KEY_DEL
#define GENERAL 7      /* Ctrl g */
#define FILES 6        /* Ctrl f */
#define SAVE 19        /* Ctrl s */
#define PREFERENCE 16  /* Ctrl p */
#define ATTRIBUTE 1    /* Ctrl a */
#define DRAW 4         /* Ctrl d */
#define OPTION 15      /* Ctrl o */
#define EXIT 5         /* Ctrl e */
#define HOME KEY_HOME  /* Home */
#define END KEY_END    /* End */
#define QUIT ESC
#define ONE KEY_F1
#define TWO KEY_F2
#define THREE KEY_F3
#define FOUR KEY_F4
#define HELPKEY KEY_F6
#define TOGGLESTAT KEY_F5
#define SPACE 32

/* the following are not related to keyboard */

/* options */
#define OPTION1 -1
#define OPTION2 -2
#define OPTION3 -3
#define OPTION4 -4

/* line modes */
#define LINE_SINGLE OPTION1
#define LINE_DOUBLE OPTION2
#define LINE_REGION -6
#define LINE_SPACE -4

/* regions */
#define NO_REGION 0
#define START_REGION 1
#define TEST_REGION 2
#define ACCEPT_REGION 3

/* switches for menus */
#define VERS 1000
#define ASCII 1001
#define FIRST -1
#define ATTRPREFERENCE 800
#define FIELDNUM 9
#define KEYS 600
#define CHOICE1 500
#define CHOICE2 501
#define CHOICE3 502
#define CHOICE4 503

#define ABS(j) (((j)>0)?(j):(-(j)))
