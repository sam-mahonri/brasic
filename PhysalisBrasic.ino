

#define kVersion "Versão 1.0"



char eliminateCompileErrors = 1;

#ifdef FORCE_DESKTOP 
#undef ARDUINO
#include "desktop.h"
#else
#define ARDUINO 1
#endif


#undef ATIVAR_FILEIO

#undef ENABLE_AUTORUN

#define kAutorunFilename  "autorun.bas"


#define ATIVAR_EAUTOINICIO 0



#undef ENABLE_TONES
#define kPiezoPin 5


#define ATIVAR_EEPROM 1


#define kConsoleBaud 9600


#ifndef RAMEND

  #ifdef ARDUINO
  
    #ifdef ESP8266
      #define RAMEND (8192-1)
    #else

      #define RAMEND (4096-1)
    #endif
  #endif
#endif



#ifdef ESP8266

  #define ALIGN_MEMORY 1
#else
  #undef ALIGN_MEMORY
#endif

#ifndef ARDUINO
 
  #undef ATIVAR_EEPROM
  #undef ENABLE_TONES
#endif



#ifdef ARDUINO

  #ifdef ATIVAR_EEPROM
    #include <EEPROM.h>  
    int eepos = 0;
  #endif

  #ifdef ATIVAR_FILEIO
    #include <SD.h>
    #include <SPI.h> 

    #define kSD_CS 10

    #define kSD_Fail  0
    #define kSD_OK    1

    File fp;
  #endif

  #ifdef ATIVAR_FILEIO
    #define kRamFileIO (1030) 
  #else
    #define kRamFileIO (0)
  #endif

  #ifdef ENABLE_TONES
    #define kRamTones (40)
  #else
    #define kRamTones (0)
  #endif

  #define kRamSize  (RAMEND - 1160 - kRamFileIO - kRamTones) 

#endif 




#ifdef ARDUINO
 
  #include <avr/pgmspace.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #undef ENABLE_TONES

 
  #define kRamSize   64*1024 

  #ifdef ATIVAR_FILEIO
    FILE * fp;
  #endif
#endif

#ifdef ALIGN_MEMORY
 
  #define ALIGN_UP(x) ((unsigned char*)(((unsigned int)(x + 1) >> 1) << 1))
  #define ALIGN_DOWN(x) ((unsigned char*)(((unsigned int)x >> 1) << 1))
#else
  #define ALIGN_UP(x) x
  #define ALIGN_DOWN(x) x
#endif




#ifndef boolean 
  #define boolean int
  #define true 1
  #define false 0
#endif

#ifndef byte
  typedef unsigned char byte;
#endif


#ifndef PROGMEM
  #define PROGMEM
#endif
#ifndef pgm_read_byte
  #define pgm_read_byte( A ) *(A)
#endif


#ifdef ATIVAR_FILEIO

  void cmd_Files( void );
  unsigned char * filenameWord(void);
  static boolean sd_is_initialized = false;
#endif



boolean inhibitOutput = false;
static boolean runAfterLoad = false;
static boolean triggerRun = false;


enum {
  kStreamSerial = 0,
  kStreamEEProm,
  kStreamFile
};
static unsigned char inStream = kStreamSerial;
static unsigned char outStream = kStreamSerial;


#define CR	'\r'
#define NL	'\n'
#define LF      0x0a
#define TAB	'\t'
#define BELL	'\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x03
#define CTRLH	0x08
#define CTRLS	0x13
#define CTRLX	0x18

typedef short unsigned LINENUM;
#ifdef ARDUINO
#define ECHO_CHARS 1
#else
#define ECHO_CHARS 0
#endif


static unsigned char program[kRamSize];
static const char *  sentinel = "";
static unsigned char *txtpos,*list_line, *tmptxtpos;
static unsigned char erro_na_expressao;
static unsigned char *tempsp;


const static unsigned char keywords[] PROGMEM = {
  'L','I','S','T','A','R'+0x80,
  'C','A','R','R','E','G','A','R'+0x80,
  'N','O','V','O'+0x80,
  'R','O','D','A','R'+0x80,
  'S','A','L','V','A','R'+0x80,
  'P','R','O','X','I','M','O' +0x80,
  'L','E','T'+0x80,
  'S','E'+0x80,
  'I','R','P','A','R','A'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','O','R','N','A','R'+0x80,
  'R','E','M'+0x80,
  'P','A','R','A'+0x80,
  'I','N','P','U','T'+0x80,
  'I','M','P','R','I','M','I','R'+0x80,
  'P','O','K','E'+0x80,
  'P','A','R','E'+0x80,
  'T','C','H','A','U'+0x80,
  'A','R','Q','U','I','V','O','S'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,
  'E','S','C','R','E','V','E','R','A'+0x80,
  'E','S','C','R','E','V','E','R','D'+0x80,
  'E','S','P','E','R','E'+0x80,
  'F','I','M'+0x80,
  'R','S','E','E','D'+0x80,
  'C','O','R','R','E','N','T','E'+0x80,
#ifdef ENABLE_TONES
  'T','O','M','W'+0x80,
  'T','O','M'+0x80,
  'S','E','M','T','O','M'+0x80,
#endif
#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
  'E','C','O','R','R','E','N','T','E'+0x80,
  'E','L','I','S','T','A','R'+0x80,
  'E','C','A','R','R','E','G','A','R'+0x80,
  'E','F','O','R','M','A','T','A','R'+0x80,
  'E','S','A','L','V','A','R'+0x80,
#endif
#endif
  0
};


enum {
  KW_LIST = 0,
  KW_LOAD, KW_NEW, KW_RUN, KW_SAVE,
  KW_NEXT, KW_LET, KW_IF,
  KW_GOTO, KW_GOSUB, KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_INPUT, KW_PRINT,
  KW_POKE,
  KW_STOP, KW_BYE,
  KW_FILES,
  KW_MEM,
  KW_QMARK, KW_QUOTE,
  KW_AWRITE, KW_DWRITE,
  KW_DELAY,
  KW_END,
  KW_RSEED,
  KW_CHAIN,
#ifdef ENABLE_TONES
  KW_TONEW, KW_TONE, KW_NOTONE,
#endif
#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
  KW_ECHAIN, KW_ELIST, KW_ELOAD, KW_EFORMAT, KW_ESAVE, 
#endif
#endif
  KW_DEFAULT 
};

struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

const static unsigned char func_tab[] PROGMEM = {
  'P','E','E','K'+0x80,
  'A','B','S'+0x80,
  'L','E','R','A'+0x80,
  'L','E','R','D'+0x80,
  'R','N','D'+0x80,
  0
};
#define FUNC_PEEK    0
#define FUNC_ABS     1
#define FUNC_AREAD   2
#define FUNC_DREAD   3
#define FUNC_RND     4
#define FUNC_UNKNOWN 5

const static unsigned char to_tab[] PROGMEM = {
  'A','T','E'+0x80,
  0
};

const static unsigned char step_tab[] PROGMEM = {
  'E','T','A','P','A'+0x80,
  0
};

const static unsigned char relop_tab[] PROGMEM = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  0
};

#define RELOP_GE		0
#define RELOP_NE		1
#define RELOP_GT		2
#define RELOP_EQ		3
#define RELOP_LE		4
#define RELOP_LT		5
#define RELOP_NE_BANG		6
#define RELOP_UNKNOWN	7

const static unsigned char highlow_tab[] PROGMEM = { 
  'A','L','T','O'+0x80,
  'A','L'+0x80,
  'B','A','I','X','O'+0x80,
  'B','A'+0x80,
  0
};
#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) 

static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack; 
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[]            PROGMEM = "OK";
static const unsigned char whatmsg[]          PROGMEM = "Erro de Sintaxe ";
static const unsigned char howmsg[]           PROGMEM =	"Faltam argumentos";
static const unsigned char sorrymsg[]         PROGMEM = "Desculpe-me.";
static const unsigned char initmsg[]          PROGMEM = "Physalis Brasic Console " kVersion;
static const unsigned char memorymsg[]        PROGMEM = " bytes livre.";
#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
static const unsigned char eeprommsg[]        PROGMEM = " EEProm bytes total.";
static const unsigned char eepromamsg[]       PROGMEM = " EEProm bytes disponivel.";
#endif
#endif
static const unsigned char breakmsg[]         PROGMEM = "Parar!";
static const unsigned char unimplimentedmsg[] PROGMEM = "Não implementado.";
static const unsigned char backspacemsg[]     PROGMEM = "\b \b";
static const unsigned char indentmsg[]        PROGMEM = "    ";
static const unsigned char sderrormsg[]       PROGMEM = "Erro no Cartão SD.";
static const unsigned char sdfilemsg[]        PROGMEM = "Erro no Arquivo SD.";
static const unsigned char dirextmsg[]        PROGMEM = "(dir)";
static const unsigned char slashmsg[]         PROGMEM = "/";
static const unsigned char spacemsg[]         PROGMEM = " ";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
static unsigned char breakcheck(void);

static void ignore_blanks(void)
{
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}



static void scantable(const unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while(1)
  {
   
    if(pgm_read_byte( table ) == 0)
      return;


    if(txtpos[i] == pgm_read_byte( table ))
    {
      i++;
      table++;
    }
    else
    {
      
      if(txtpos[i]+0x80 == pgm_read_byte( table ))
      {
        txtpos += i+1;  
        ignore_blanks();
        return;
      }


      while((pgm_read_byte( table ) & 0x80) == 0)
        table++;

     
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}


static void pushb(unsigned char b)
{
  sp--;
  *sp = b;
}


static unsigned char popb()
{
  unsigned char b;
  b = *sp;
  sp++;
  return b;
}

void printnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

void printUnum(unsigned int num)
{
  int digits = 0;

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}


static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return	num;
}

static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; 

  return 1;
}



void imprimirmsgNoNL(const unsigned char *msg)
{
  while( pgm_read_byte( msg ) != 0 ) {
    outchar( pgm_read_byte( msg++ ) );
  };
}

void imprimirmsg(const unsigned char *msg)
{
  imprimirmsgNoNL(msg);
  line_terminator();
}


static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      line_terminator();

      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      imprimirmsg(backspacemsg);
      break;
    default:
     
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}


static unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while(1)
  {
    if(line == program_end)
      return line;

    if(((LINENUM *)line)[0] >= linenum)
      return line;


    line += line[sizeof(LINENUM)];
  }
}


static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
   
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}


void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);


  printnum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
#ifdef ALIGN_MEMORY
  
  if (ALIGN_UP(list_line) != list_line)
    list_line++;
#endif
  line_terminator();
}

static short int expr4(void)
{

  ignore_blanks();

  if( *txtpos == '-' ) {
    txtpos++;
    return -expr4();
  }


  if(*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if(*txtpos >= '1' && *txtpos <= '9')
  {
    short int a = 0;
    do 	{
      a = a*10 + *txtpos - '0';
      txtpos++;
    } 
    while(*txtpos >= '0' && *txtpos <= '9');
    return a;
  }


  if(txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    short int a;

    if(txtpos[1] < 'A' || txtpos[1] > 'Z')
    {
      a = ((short int *)variables_begin)[*txtpos - 'A'];
      txtpos++;
      return a;
    }


    scantable(func_tab);
    if(table_index == FUNC_UNKNOWN)
      goto expr4_error;

    unsigned char f = table_index;

    if(*txtpos != '(')
      goto expr4_error;

    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;
    txtpos++;
    switch(f)
    {
    case FUNC_PEEK:
      return program[a];
      
    case FUNC_ABS:
      if(a < 0) 
        return -a;
      return a;

#ifdef ARDUINO
    case FUNC_AREAD:
      pinMode( a, INPUT );
      return analogRead( a );                        
    case FUNC_DREAD:
      pinMode( a, INPUT );
      return digitalRead( a );
#endif

    case FUNC_RND:
#ifdef ARDUINO
      return( random( a ));
#else
      return( rand() % a );
#endif
    }
  }

  if(*txtpos == '(')
  {
    short int a;
    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  erro_na_expressao = 1;
  return 0;

}


static short int expr3(void)
{
  short int a,b;

  a = expr4();

  ignore_blanks(); 

  while(1)
  {
    if(*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if(*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if(b != 0)
        a /= b;
      else
        erro_na_expressao = 1;
    }
    else
      return a;
  }
}

static short int expr2(void)
{
  short int a,b;

  if(*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while(1)
  {
    if(*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if(*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}

static short int expression(void)
{
  short int a,b;

  a = expr2();

 
  if(erro_na_expressao)	return a;

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
    return a;

  switch(table_index)
  {
  case RELOP_GE:
    b = expr2();
    if(a >= b) return 1;
    break;
  case RELOP_NE:
  case RELOP_NE_BANG:
    b = expr2();
    if(a != b) return 1;
    break;
  case RELOP_GT:
    b = expr2();
    if(a > b) return 1;
    break;
  case RELOP_EQ:
    b = expr2();
    if(a == b) return 1;
    break;
  case RELOP_LE:
    b = expr2();
    if(a <= b) return 1;
    break;
  case RELOP_LT:
    b = expr2();
    if(a < b) return 1;
    break;
  }
  return 0;
}

void loop()
{
  
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;
  boolean isDigital;
  boolean alsoWait = false;
  int val;

#ifdef ARDUINO
#ifdef ENABLE_TONES
  noTone( kPiezoPin );
#endif
#endif

  program_start = program;
  program_end = program_start;
  sp = program+sizeof(program); 
#ifdef ALIGN_MEMORY

  stack_limit = ALIGN_DOWN(program+sizeof(program)-STACK_SIZE);
  variables_begin = ALIGN_DOWN(stack_limit - 27*VAR_SIZE);
#else
  stack_limit = program+sizeof(program)-STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;
#endif

 
  printnum(variables_begin-program_end);
  imprimirmsg(memorymsg);
#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
  
  printnum( E2END+1 );
  imprimirmsg( eeprommsg );
#endif 
#endif 

warmstart:
  
  current_line = 0;
  sp = program+sizeof(program);
  imprimirmsg(okmsg);

prompt:
  if( triggerRun ){
    triggerRun = false;
    current_line = program_start;
    goto execline;
  }

  getln( '>' );
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);

   
  while(*txtpos != NL)
    txtpos++;

 
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; 
  linelen += sizeof(unsigned short)+sizeof(char); 

  
  txtpos -= 3;

#ifdef ALIGN_MEMORY
  
  if (ALIGN_DOWN(txtpos) != txtpos)
  {
    txtpos--;
    linelen++;
    
    unsigned char *tomove;
    tomove = txtpos + 3;
    while (tomove < txtpos + linelen - 1)
    {
      *tomove = *(tomove + 1);
      tomove++;
    }
  }
#endif

  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


 
  start = findline();

  
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    }	
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) 
    goto prompt;



 
  while(linelen > 0)
  {	
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;


   
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

   
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  imprimirmsg(unimplimentedmsg);
  goto prompt;

qhow:	
  imprimirmsg(howmsg);
  goto prompt;

qerro:	
  imprimirmsgNoNL(whatmsg);
  if(current_line != NULL)
  {
    unsigned char tmp = *txtpos;
    if(*txtpos != NL)
      *txtpos = '^';
    list_line = current_line;
    printline();
    *txtpos = tmp;
  }
  line_terminator();
  goto prompt;

qsorry:	
  imprimirmsg(sorrymsg);
  goto warmstart;

run_next_statement:
  while(*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if(*txtpos == NL)
    goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
  if(breakcheck())
  {
    imprimirmsg(breakmsg);
    goto warmstart;
  }

  scantable(keywords);

  switch(table_index)
  {
  case KW_DELAY:
    {
#ifdef ARDUINO
      erro_na_expressao = 0;
      val = expression();
      delay( val );
      goto execnextline;
#else
      goto unimplemented;
#endif
    }

  case KW_FILES:
    goto files;
  case KW_LIST:
    goto list;
  case KW_CHAIN:
    goto chain;
  case KW_LOAD:
    goto load;
  case KW_MEM:
    goto mem;
  case KW_NEW:
    if(txtpos[0] != NL)
      goto qerro;
    program_end = program_start;
    goto prompt;
  case KW_RUN:
    current_line = program_start;
    goto execline;
  case KW_SAVE:
    goto save;
  case KW_NEXT:
    goto next;
  case KW_LET:
    goto assignment;
  case KW_IF:
    short int val;
    erro_na_expressao = 0;
    val = expression();
    if(erro_na_expressao || *txtpos == NL)
      goto qhow;
    if(val != 0)
      goto interperateAtTxtpos;
    goto execnextline;

  case KW_GOTO:
    erro_na_expressao = 0;
    linenum = expression();
    if(erro_na_expressao || *txtpos != NL)
      goto qhow;
    current_line = findline();
    goto execline;

  case KW_GOSUB:
    goto gosub;
  case KW_RETURN:
    goto gosub_return; 
  case KW_REM:
  case KW_QUOTE:
    goto execnextline;	
  case KW_FOR:
    goto forloop; 
  case KW_INPUT:
    goto input; 
  case KW_PRINT:
  case KW_QMARK:
    goto print;
  case KW_POKE:
    goto poke;
  case KW_END:
  case KW_STOP:
    
    if(txtpos[0] != NL)
      goto qerro;
    current_line = program_end;
    goto execline;
  case KW_BYE:
   
    return;

  case KW_AWRITE: 
    isDigital = false;
    goto awrite;
  case KW_DWRITE:
    isDigital = true;
    goto dwrite;

  case KW_RSEED:
    goto rseed;

#ifdef ENABLE_TONES
  case KW_TONEW:
    alsoWait = true;
  case KW_TONE:
    goto tonegen;
  case KW_NOTONE:
    goto tonestop;
#endif

#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
  case KW_EFORMAT:
    goto eformat;
  case KW_ESAVE:
    goto esave;
  case KW_ELOAD:
    goto eload;
  case KW_ELIST:
    goto elist;
  case KW_ECHAIN:
    goto echain;
#endif
#endif

  case KW_DEFAULT:
    goto assignment;
  default:
    break;
  }

execnextline:
  if(current_line == NULL)	
    goto prompt;
  current_line +=	 current_line[sizeof(LINENUM)];

execline:
  if(current_line == program_end) 
    goto warmstart;
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;

#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
elist:
  {
    int i;
    for( i = 0 ; i < (E2END +1) ; i++ )
    {
      val = EEPROM.read( i );

      if( val == '\0' ) {
        goto execnextline;
      }

      if( ((val < ' ') || (val  > '~')) && (val != NL) && (val != CR))  {
        outchar( '?' );
      } 
      else {
        outchar( val );
      }
    }
  }
  goto execnextline;

eformat:
  {
    for( int i = 0 ; i < E2END ; i++ )
    {
      if( (i & 0x03f) == 0x20 ) outchar( '.' );
      EEPROM.write( i, 0 );
    }
    outchar( LF );
  }
  goto execnextline;

esave:
  {
    outStream = kStreamEEProm;
    eepos = 0;


    list_line = findline();
    while(list_line != program_end) {
      printline();
    }
    outchar('\0');

  
    outStream = kStreamSerial;
    
    goto warmstart;
  }
  
  
echain:
  runAfterLoad = true;

eload:
 
  program_end = program_start;

 
  eepos = 0;
  inStream = kStreamEEProm;
  inhibitOutput = true;
  goto warmstart;
#endif
#endif

input:
  {
    unsigned char var;
    int value;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qerro;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qerro;
inputagain:
    tmptxtpos = txtpos;
    getln( '?' );
    toUppercaseBuffer();
    txtpos = program_end+sizeof(unsigned short);
    ignore_blanks();
    erro_na_expressao = 0;
    value = expression();
    if(erro_na_expressao)
      goto inputagain;
    ((short int *)variables_begin)[var-'A'] = value;
    txtpos = tmptxtpos;

    goto run_next_statement;
  }

forloop:
  {
    unsigned char var;
    short int initial, step, terminal;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qerro;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '=')
      goto qerro;
    txtpos++;
    ignore_blanks();

    erro_na_expressao = 0;
    initial = expression();
    if(erro_na_expressao)
      goto qerro;

    scantable(to_tab);
    if(table_index != 0)
      goto qerro;

    terminal = expression();
    if(erro_na_expressao)
      goto qerro;

    scantable(step_tab);
    if(table_index == 0)
    {
      step = expression();
      if(erro_na_expressao)
        goto qerro;
    }
    else
      step = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qerro;


    if(!erro_na_expressao && *txtpos == NL)
    {
      struct stack_for_frame *f;
      if(sp + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp;
      ((short int *)variables_begin)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  erro_na_expressao = 0;
  linenum = expression();
  if(!erro_na_expressao && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(sp + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
 
  ignore_blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_blanks();
  if(*txtpos != ':' && *txtpos != NL)
    goto qerro;

gosub_return:

  tempsp = sp;
  while(tempsp < program+sizeof(program)-1)
  {
    switch(tempsp[0])
    {
    case STACK_GOSUB_FLAG:
      if(table_index == KW_RETURN)
      {
        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
        current_line	= f->current_line;
        txtpos			= f->txtpos;
        sp += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
    
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
    
      if(table_index == KW_NEXT)
      {
        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
     
        if(txtpos[-1] == f->for_var)
        {
          short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A'; 
          *varaddr = *varaddr + f->step;
          
          if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
          {

            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
         
          sp = tempsp + sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
     
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:
      
      goto warmstart;
    }
  }
 
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;

    ignore_blanks();

    if (*txtpos != '=')
      goto qerro;
    txtpos++;
    ignore_blanks();
    erro_na_expressao = 0;
    value = expression();
    if(erro_na_expressao)
      goto qerro;
    
    if(*txtpos != NL && *txtpos != ':')
      goto qerro;
    *var = value;
  }
  goto run_next_statement;
poke:
  {
    short int value;
    unsigned char *address;

   
    erro_na_expressao = 0;
    value = expression();
    if(erro_na_expressao)
      goto qerro;
    address = (unsigned char *)value;

   
    ignore_blanks();
    if (*txtpos != ',')
      goto qerro;
    txtpos++;
    ignore_blanks();

   
    erro_na_expressao = 0;
    value = expression();
    if(erro_na_expressao)
      goto qerro;
   
    if(*txtpos != NL && *txtpos != ':')
      goto qerro;
  }
  goto run_next_statement;

list:
  linenum = testnum(); 

  
  if(txtpos[0] != NL)
    goto qerro;

  
  list_line = findline();
  while(list_line != program_end)
    printline();
  goto warmstart;

print:
 
  if(*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(print_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qerro;
    else
    {
      short int e;
      erro_na_expressao = 0;
      e = expression();
      if(erro_na_expressao)
        goto qerro;
      printnum(e);
    }

  
    if(*txtpos == ',')
      txtpos++;	
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; 
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();
      break;
    }
    else
      goto qerro;	
  }
  goto run_next_statement;

mem:

  printnum(variables_begin-program_end);
  imprimirmsg(memorymsg);
#ifdef ARDUINO
#ifdef ATIVAR_EEPROM
  {
   
    printnum( E2END+1 );
    imprimirmsg( eeprommsg );
    
   
    val = ' ';
    int i;   
    for( i=0 ; (i<(E2END+1)) && (val != '\0') ; i++ ) {
      val = EEPROM.read( i );    
    }
    printnum( (E2END +1) - (i-1) );
    
    imprimirmsg( eepromamsg );
  }
#endif 
#endif 
  goto run_next_statement;


 

#ifdef ARDUINO
awrite: 
dwrite:
  {
    short int pinNo;
    short int value;
    unsigned char *txtposBak;

   
    erro_na_expressao = 0;
    pinNo = expression();
    if(erro_na_expressao)
      goto qerro;

  
    ignore_blanks();
    if (*txtpos != ',')
      goto qerro;
    txtpos++;
    ignore_blanks();


    txtposBak = txtpos; 
    scantable(highlow_tab);
    if(table_index != HIGHLOW_UNKNOWN)
    {
      if( table_index <= HIGHLOW_HIGH ) {
        value = 1;
      } 
      else {
        value = 0;
      }
    } 
    else {

     
      erro_na_expressao = 0;
      value = expression();
      if(erro_na_expressao)
        goto qerro;
    }
    pinMode( pinNo, OUTPUT );
    if( isDigital ) {
      digitalWrite( pinNo, value );
    } 
    else {
      analogWrite( pinNo, value );
    }
  }
  goto run_next_statement;
#else
pinmode: 
awrite: 
dwrite:
  goto unimplemented;
#endif

  
files:

 

#ifdef ATIVAR_FILEIO
    cmd_Files();
  goto warmstart;
#else
  goto unimplemented;
#endif 


chain:
  runAfterLoad = true;

load:
 
  program_end = program_start;

  
#ifdef ATIVAR_FILEIO
  {
    unsigned char *filename;

    
    erro_na_expressao = 0;
    filename = filenameWord();
    if(erro_na_expressao)
      goto qerro;

#ifdef ARDUINO
   
    if( !SD.exists( (char *)filename ))
    {
      imprimirmsg( sdfilemsg );
    } 
    else {

      fp = SD.open( (const char *)filename );
      inStream = kStreamFile;
      inhibitOutput = true;
    }
#else 
  
#endif 
   

  }
  goto warmstart;
#else 
  goto unimplemented;
#endif



save:
 
#ifdef ATIVAR_FILEIO
  {
    unsigned char *filename;

   
    erro_na_expressao = 0;
    filename = filenameWord();
    if(erro_na_expressao)
      goto qerro;

#ifdef ARDUINO
   
    if( SD.exists( (char *)filename )) {
      SD.remove( (char *)filename );
    }

    
    fp = SD.open( (const char *)filename, FILE_WRITE );
    outStream = kStreamFile;

  
    list_line = findline();
    while(list_line != program_end)
      printline();

    
    outStream = kStreamSerial;

    fp.close();
#else 

#endif
    goto 
  }
#else 
  goto unimplemented;
#endif 
rseed:
  {
    short int value;

   
    erro_na_expressao = 0;
    value = expression();
    if(erro_na_expressao)
      goto qerro;

#ifdef ARDUINO
    randomSeed( value );
#else
    srand( value );
#endif 
    goto run_next_statement;
  }

#ifdef ENABLE_TONES
tonestop:
  noTone( kPiezoPin );
  goto run_next_statement;

tonegen:
  {
   
    short int freq;
    short int duracao;

   
    erro_na_expressao = 0;
    freq = expression();
    if(erro_na_expressao)
      goto qerro;

    ignore_blanks();
    if (*txtpos != ',')
      goto qerro;
    txtpos++;
    ignore_blanks();


   
    erro_na_expressao = 0;
    duracao = expression();
    if(erro_na_expressao)
      goto qerro;

    if( freq == 0 || duracao == 0 )
      goto tonestop;

    tone( kPiezoPin, freq, duracao );
    if( alsoWait ) {
      delay( duracao );
      alsoWait = false;
    }
    goto run_next_statement;
  }
#endif ENABLE_TONES
}


static int isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; 
  if( c >= 'A' && c <= 'Z' ) return 1; 
  if( c >= 'a' && c <= 'z' ) return 1; 
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  

  return 0;
}

unsigned char * filenameWord(void)
{
  
  unsigned char * ret = txtpos;
  erro_na_expressao = 0;

  
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    erro_na_expressao = 1;
    return ret;
  }

  
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

 
  if( *ret == '\0' ) {
    erro_na_expressao = 1;
  }

  return ret;
}


static void line_terminator(void)
{
  outchar(NL);
  outchar(CR);
}


void setup()
{
#ifdef ARDUINO
  Serial.begin(kConsoleBaud);	
  while( !Serial ); 
  
  Serial.println( sentinel );
  imprimirmsg(initmsg);

#ifdef ATIVAR_FILEIO
  initSD();
  
#ifdef ENABLE_AUTORUN
  if( SD.exists( kAutorunFilename )) {
    program_end = program_start;
    fp = SD.open( kAutorunFilename );
    inStream = kStreamFile;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif 

#endif 

/* #ifdef ATIVAR_EEPROM
#ifdef ATIVAR_EAUTOINICIO
  
  int val = EEPROM.read(0);
  if( val >= '0' && val <= '9' ) {
    program_end = program_start;
    inStream = kStreamEEProm;
    eepos = 0;
    inhibitOutput = true;
    runAfterLoad = false;
  }
#endif 
#endif */

#endif 
}



static unsigned char breakcheck(void)
{
#ifdef ARDUINO
  if(Serial.available())
    return Serial.read() == CTRLC;
  return 0;
#else
#ifdef __CONIO__
  if(kbhit())
    return getch() == CTRLC;
  else
#endif
    return 0;
#endif
}

static int inchar()
{
  int v;
#ifdef ARDUINO
  
  switch( inStream ) {
  case( kStreamFile ):
#ifdef ATIVAR_FILEIO
    v = fp.read();
    if( v == NL ) v=CR;
    if( !fp.available() ) {
      fp.close();
      goto inchar_loadfinish;
    }
    return v;    
#else
#endif
     break;
  case( kStreamEEProm ):
#ifdef ATIVAR_EEPROM
#ifdef ARDUINO
    v = EEPROM.read( eepos++ );
    if( v == '\0' ) {
      goto inchar_loadfinish;
    }
    return v;
#endif
#else
    inStream = kStreamSerial;
    return NL;
#endif
     break;
  case( kStreamSerial ):
  default:
    while(1)
    {
      if(Serial.available())
        return Serial.read();
    }
  }
  
inchar_loadfinish:
  inStream = kStreamSerial;
  inhibitOutput = false;

  if( runAfterLoad ) {
    runAfterLoad = false;
    triggerRun = true;
  }
  return NL; 
  
#else
  
  int got = getchar();

  
  if( got == LF ) got = CR;

  return got;
#endif
}


static void outchar(unsigned char c)
{
  if( inhibitOutput ) return;

#ifdef ARDUINO
  #ifdef ATIVAR_FILEIO
    if( outStream == kStreamFile ) {
     
      fp.write( c );
    } 
    else
  #endif
  #ifdef ARDUINO
  #ifdef ATIVAR_EEPROM
    if( outStream == kStreamEEProm ) {
      EEPROM.write( eepos++, c );
    }
    else 
  #endif /* ATIVAR_EEPROM */
  #endif /* ARDUINO */
    Serial.write(c);

#else
  putchar(c);
#endif
}




#if ARDUINO && ATIVAR_FILEIO

static int initSD( void )
{
  

  if( sd_is_initialized == true ) return kSD_OK;

  
  pinMode(10, OUTPUT); 

  if( !SD.begin( kSD_CS )) {
    
    imprimirmsg( sderrormsg );
    return kSD_Fail;
  }
 
  sd_is_initialized = true;

 
  outStream = kStreamSerial;
  inStream = kStreamSerial;
  inhibitOutput = false;

  return kSD_OK;
}
#endif

#if ATIVAR_FILEIO
void cmd_Files( void )
{
  File dir = SD.open( "/" );
  dir.seek(0);

  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }

  
    imprimirmsgNoNL( indentmsg );
    imprimirmsgNoNL( (const unsigned char *)entry.name() );
    if( entry.isDirectory() ) {
      imprimirmsgNoNL( slashmsg );
    }

    if( entry.isDirectory() ) {
      
      for( int i=strlen( entry.name()) ; i<16 ; i++ ) {
        imprimirmsgNoNL( spacemsg );
      }
      imprimirmsgNoNL( dirextmsg );
    }
    else {
     
      for( int i=strlen( entry.name()) ; i<17 ; i++ ) {
        imprimirmsgNoNL( spacemsg );
      }
      printUnum( entry.size() );
    }
    line_terminator();
    entry.close();
  }
  dir.close();
}
#endif
