

#include <peekpoke.h>
#include <cbm.h>
#include <conio.h>

typedef unsigned int  word;
typedef unsigned char byte;

#define VERSION_ADDR	0xff80

#define BLACK           0x90
#define WHITE           0x05            
#define RED             0x1c
#define GREEN           0x1e
#define BLUE            0x1f
#define AMBER           0x81
#define LTRED           0x96
#define LTGREEN         0x99
#define LTBLUE          0x9a
#define YELLOW          0x9e
#define LTGREY          155
#define PURPLE          156
#define CYAN            159

#define TO_BACKGROUND   0x01

#define RVS_ON          18
#define RVS_OFF         146

word address = 0xfe00;
byte ramBank = 0;
word bankCount;

byte version;
byte modVersion;

byte row,col;
byte c;
char  petscii[16];
char* release;
byte color;

byte colorPalette[16] =
	{
	PURPLE,
	YELLOW,
	GREEN,
	LTGREY,	
	PURPLE,
	YELLOW,
	GREEN,
	LTGREY,	
	PURPLE,
	YELLOW,
	GREEN,
	LTGREY,	
	PURPLE,
	YELLOW,
	GREEN,
	LTGREY	
	};

void status()
{
   gotoxy(1,1);

   cbm_k_bsout(LTGREY);
   cprintf("%5u    ram bank (a): %3u / %3u total                  %s rev %u kernal\r\n", 
	address,
	ramBank,
        bankCount,
        release,
        modVersion
        );

   gotoxy(0,3);
   cputs("       00 01 02 03 04 05 06 07   08 09 0a 0b 0c 0d 0e 0f\r\n\r\n");

   cbm_k_bsout(color);
}

void hexdump()
{
   word currentAddress;

   for(row=0;row<48;++row)
   {
      gotoxy(1,row+5);
      cprintf("%04x  ", address + row * 16);
      for(col=0;col<16;++col)
      {
         currentAddress = address + row * 16 + col;
         c = PEEK(currentAddress);

         if ((currentAddress == VERSION_ADDR)
	   || currentAddress == 0)
            revers(1);

         if ( currentAddress > 1 
           && currentAddress < 18)
            cbm_k_bsout(colorPalette[currentAddress-2]);

         cprintf("%02x ", c);

         if ( currentAddress > 1 
           && currentAddress < 18)
            cbm_k_bsout(color);

         if ((currentAddress == VERSION_ADDR)
	   || currentAddress == 0)
            revers(0);

         if (col == 7) 
         {
            cputc(' ');
            cputc(' ');
         }
         petscii[col] = '.';
         if (c > 31 && c < 128)
            petscii[col] = c; 
      }
      for(col=0;col<16;++col)
      {
         currentAddress = address + row * 16 + col;
         if ( currentAddress > 0xfff5
           && currentAddress < 0xfffa )
            revers(1);

         cputc(petscii[col]);
         if (col == 7)
            cputc(' ');

         if ( currentAddress > 0xfff5
           && currentAddress < 0xfffa )
            revers(0);
      }
   }
}

void controls()
{
   cbm_k_bsout(LTGREY);
   cputsxy(1,58,"left/right $100; up/down $300; return/arrow $1000 ");
   cbm_k_bsout(color);
}

void screen()
{
   status();
   hexdump();
   controls();
}

void setRAMbank(byte bankNum)
{
   if (version > 217)
      POKE(0x9f61,bankNum);
   else
      POKE(0,bankNum);	
}

void setColor(byte col)
{
   color = col;
}

void command()
{
   switch(cgetc())
   { 
      case 0x9d: // right
	address -= 0x100;
	break;

      case 0x1d: // left
	address += 0x100;
	break;

      case 0x91: // up
	address -= 0x300; 
	break;

      case 0x11: // down
	address += 0x300;
	break;

      case 13:   // enter
        address += 0x1000;
        break;

      case 95:   // left arrow
        address -= 0x1000;
        break;

      case 'a':  // RAM bank ++
	++ramBank;
        setRAMbank(ramBank);
        status();
        if (address <= 0xa000 - 0x300
         && address >= 0xc000)
           return;
        break;

      case 193:  // RAM bank --
	--ramBank;
        setRAMbank(ramBank);
        status();
        if (address <= 0xa000 - 0x300
         && address >= 0xc000)
           return;
        break;

      case '1': setColor(WHITE);   break;
      case '2': setColor(LTRED);   break;
      case '3': setColor(CYAN);    break;
      case '4': setColor(PURPLE);  break;
      case '5': setColor(GREEN);   break;
      case '6': setColor(LTBLUE);  break;
      case '7': setColor(YELLOW);  break;
      case '8': setColor(AMBER);   break;
      case '9': setColor(LTGREEN); break;
      case '0': setColor(LTGREY);  break;
   }
   screen();
}

void getVersion()
{
   version = PEEK(VERSION_ADDR);
   if (version > 127)
   {
      release = "proto";
      modVersion = 0x100 - version;
   }
   else
   {
      release = "release";
      modVersion = version;
   }
}

void determineBankCount()
{
   int i=0;

   setRAMbank(1);    
   POKE(0xa000,17);

   bankCount = 256;

   setRAMbank(193);
   if (PEEK(0xa000) == 17) bankCount = 192;

   setRAMbank(129);
   if (PEEK(0xa000) == 17) bankCount = 128;

   setRAMbank(65);
   if (PEEK(0xa000) == 17) bankCount = 64;
}

void main(void)
{
   getVersion();
   determineBankCount();

   cbm_k_bsout(0x8E); // revert to primary case

   cbm_k_bsout(BLACK);
   cbm_k_bsout(TO_BACKGROUND);
   
   setColor(LTBLUE);

   clrscr();

   cbm_k_setnam("petfont.bin");
   cbm_k_setlfs(0,8,0);
   cbm_k_load(2, 0x0f800);

   screen();
   controls();

   while(1)
   {
      command();
   }
}

