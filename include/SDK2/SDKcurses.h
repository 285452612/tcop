/********************************************/
/*    SDK means SUNLAN's DEVELOPMENT KIT    */
/*    created by SUNLAN 2001-02-10          */
/********************************************/

#ifndef _SDKCURSES_H
#define _SDKCURSES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <menu.h>
#include <form.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "SDKbool.h"

#define PRT_DEV stdout

#ifndef CTRL
#define CTRL(x)     ((x) & 0x1f)
#endif

#define ESCAPE      CTRL('[')
#define QUIT        CTRL('Q')
#define FORM_QUIT   MAX_FORM_COMMAND+1

#define UKEY_ESCAPE 27
#define UKEY_ENTER  13
#define UKEY_DEL    127

#define UCOMMAND_NEXT_FIELD REQ_NEXT_FIELD  /* move to next field */
#define UCOMMAND_PREV_FIELD REQ_PREV_FIELD  /* move to previous field */
#define UCOMMAND_FIRST_FIELD    REQ_FIRST_FIELD /* move to first field */
#define UCOMMAND_LAST_FIELD REQ_LAST_FIELD  /* move to last field */
#define UCOMMAND_SNEXT_FIELD    REQ_SNEXT_FIELD /* move to sorted next field */
#define UCOMMAND_SPREV_FIELD    REQ_SPREV_FIELD /* move to sorted prev field */
#define UCOMMAND_SFIRST_FIELD   REQ_SFIRST_FIELD/* move to sorted first field */
#define UCOMMAND_SLAST_FIELD    REQ_SLAST_FIELD /* move to sorted last field */
#define UCOMMAND_LEFT_FIELD REQ_LEFT_FIELD  /* move to left to field */
#define UCOMMAND_RIGHT_FIELD    REQ_RIGHT_FIELD /* move to right to field */
#define UCOMMAND_UP_FIELD   REQ_UP_FIELD    /* move to up to field */
#define UCOMMAND_DOWN_FIELD REQ_DOWN_FIELD  /* move to down to field */

#define UCOMMAND_QUIT   FORM_QUIT
#define UCOMMAND_LIST   MAX_FORM_COMMAND+100
#define UCOMMAND_KBOARD MAX_FORM_COMMAND+101


typedef struct ch_cn
{
    bool iswch;
    unsigned int h_char;
    unsigned int l_char;
} ch_cn;


/*********************************************
    MENU defines
*********************************************/

#define ITEM_NAME_MAX   20
#define ITEM_DESC_MAX   40

#define ITEM_LEVEL_0    0   /* no sub menu */
#define ITEM_LEVEL_1    1   /* have sub menu */
#define ITEM_SEPARATOR  -1  /* separate item */

typedef struct
{
    unsigned int id;
    char name[ITEM_NAME_MAX+1];
    char discrep[ITEM_DESC_MAX+1];
    unsigned int level;
    void (*action)();   /*action when enter pressed*/
} SDKITEM;

typedef struct SDKMENU
{
    WINDOW *win;        /*menu is in this window*/
    int begin_y,begin_x;    /*menu's top_left conner position*/
    int item_num;       /*menu's item number*/
    int item_curr;      /*current item index of the menu*/
    ITEM **items;
    MENU *border;
    SDKITEM *iteminfo;
    struct SDKMENU *parent; /*submenu's parent,
                if it's mainmenu parent==NULL
                */
} SDKMENU;


#define MENU_QUIT   MAX_MENU_COMMAND+1
#define MENU_SELECT     MAX_MENU_COMMAND+2
#define REQ_P_LEFT_ITEM     MAX_MENU_COMMAND+3  /* left item of parent
                             menu*/
#define REQ_P_RIGHT_ITEM    MAX_MENU_COMMAND+4  /* right item of parent
                            menu */

/**********************************************
    FORM defines
**********************************************/
#define LABEL_NAME_MAX  16
#define FIELD_DESC_MAX  40

#define T_NORMAL    0
#define T_ALPHA     1   /*TYPE_ALPHA*/
#define T_ALNUM     2   /*TYPE_ALNUM*/
#define T_ENUM      3   /*TYPE_ENUM*/
#define T_INTEGER   4   /*TYPE_INTEGER*/
#define T_NUMERIC   5   /*TYPE_NUMERIC*/
#define T_REGEXP    6   /*TYPE_REGEXP*/
#define T_IPV4      7   /*TYPE_IPV4*/

#define ENCRYPT     1
#define NOEDIT      2

typedef struct
{
    int begin_y, begin_x;
    int h,w;                /*height,width*/
    int type;               /* see T_xxxxx definations */
    int opts;               /*0: Normal 1: ENCRYPT 2: Ineditable*/
    char **chklist;
    char desc[FIELD_DESC_MAX+1];
} SDKFIELD;

typedef struct
{
    char name[LABEL_NAME_MAX+1];
    int begin_x,begin_y;
} SDKLABEL;

typedef struct
{
    int begin_y, begin_x;
    int h, w;   /*height,width*/
    FORM *body;
    WINDOW *win;
    SDKFIELD *fields;
    int fld_num;
    SDKLABEL *lables;
    int lbl_num;
} SDKFORM;


#define MAX_SCROLL_LEN  65

typedef struct
{
    int rownum;
    char **rows;
} SDKSCRLBUFF;

/*Device operations*/
#ifdef HP
#define TCGETA  TCGETATTR
#define TCSETA  TCSETATTR
#endif

struct Card_info
{
    char cardno[21];
    char track2[256];
    char track3[256];
    char offset[7];
};


/*Msgbox botton type*/
#define MSGBOX_OKONLY           0
#define MSGBOX_OKCANCEL         1
#define MSGBOX_RETRYCANCEL      2
#define MSGBOX_OKRETRYCANCEL        3

/*MsgBox return code*/
#define MSG_OK              0
#define MSG_CANCEL          1
#define MSG_RETRY           2


/*Listbox definations*/
#define LISTBOX_H       5   /*display 5 list item*/
#define LISTBOX_W       20  /*list item's max length*/

typedef struct {
    char text[LISTBOX_W+1];
    union {
      int i_val;
      char c_val[40];
    } val;
} LISTITEM;

/*****************************************************
    以下为可调用函数列表
*****************************************************/

/*****************************************************
    FORM functions
*****************************************************/

int SDKwTitle( WINDOW *win, int row, const char *title );

FORM *SDKdisplay_form( int x, int y, int h, int w,
    SDKFIELD *fields, int fld_num,
    SDKLABEL *labels, int lb_num );
/************************************************
    x,y:    top_left corner
    h,w:    form's height and width
    fields: pointer to fields array
    fld_num:field items
    lables: pointer to lables
    lb_num: lable items
************************************************/

SDKLABEL** SDKInitLabel( SDKLABEL *labels, int n );

SDKFIELD** SDKInitField( SDKFIELD *fields, int n );

FORM *SDKNewForm( SDKFIELD **fields );

int SDKRunForm( SDKFORM *sform );

FORM *SDKCurrentForm();
#define CURRENT_FORM    (FORM *)SDKCurrentForm()

int SDKSetCurrentForm( FORM *form );

void SDKFormHLine( int y, int x, chtype line_type, int length );

int SDKedit_field( int fieldid, char *buffer );
/************************************************
    fieldid:field id in a filed array. begin with 0
    buffer: edit buffer with field
************************************************/

void SDKFieldValidate( int command, bool flag );

void SDKfrmsetval( FORM *frm, int fldid, const char *val );
void SDKSetFieldVal( int fieldid, const char *val );


int SDKCurrentField();
#define CURRENT_FIELD   (int)SDKCurrentField()

int SDKSetCurrentField( int fldid );

void SDKfrmrefresh( FORM *frm );

void SDKfldEnable( FORM *frm, int fldid, bool flag );
#define SDKFieldEnable( fld, flag )  SDKfldEnable( CURRENT_FORM, fld, flag )

void SDKfldVisible( FORM *frm, int fldid, bool flag );
#define SDKFieldVisible( fld, flag ) SDKfldVisible( CURRENT_FORM, fld, flag )

void SDKLabVisible( FORM *frm, SDKLABEL *labels, int labid, bool flag );

#define SDKfTitle( f,l,s ) SDKwTitle( form_sub(f),l,s )
#define SDKwprompt( w,l,s ) SDKwTitle(w,l,s)
#define SDKfprompt( f,l,s ) SDKwTitle( form_sub(f),l,s )
#define SDKFormPrompt( l, s )   SDKfprompt( CURRENT_FORM, l, s )

#define ShellMode() endwin()
#define MAXSCREEN( y, x ) getmaxyx( stdscr, y, x )
#define SDKfrmgetch( f ) wgetch( form_sub(f) )
#define SDKscrmaxyx( r,c ) getmaxyx( stdscr,r,c )
#define SDKfrmmaxyx( f,r,c ) getmaxyx( form_win(f),r,c )

/**************************************************
    MENU functions
**************************************************/

int SDKRunMenu( SDKMENU * );
int SDKFreeMenu( SDKMENU * );
int SDKCurrentMenuItem();


/*******************************************************
    other functions
*******************************************************/

int SDKi_zh( struct Card_info *card );
int KeypadOn();
int KeypadOff();

int SDKmsgbox( char *title, char *prompt, int type, int h, int w );

/*
int SDKlistbox( char *list[LISTBOX_W+1], int n, int y, int x, int defid, char *outstr );
*/
int SDKlistbox( LISTITEM *list, int n, int y, int x, int defid );
int GetListItemNo( LISTITEM *list, int listnum, int listval );
int GetListItemText( LISTITEM *list, int listnum, int listval, char *text );

#endif          /* _SDKCURSES_H */
