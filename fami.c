
/*
 * Simple Hex Dump, memory R/W tool for NES alike units
 * Aim was to create something to allow device hardware
 * exploration when all you have is the screen and the
 * joystick/A/B/START/SELECT buttons.
 *
 * Utilizes neslib, and CC65 compiler libraries to make
 * coding simpler.
 *
 */

// NESlib : https://github.com/sehugg/neslib
#include "neslib.h"

// CC65 : https://github.com/cc65/cc65/blob/master/include/joystick.h
#include <joystick.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// setup Famitone library

//#link "famitone2.s"
//void __fastcall__ famitone_update(void);
//#link "music_aftertherain.s"
//extern char after_the_rain_music_data[];
//#link "music_dangerstreets.s"
//extern char danger_streets_music_data[];
//#link "demosounds.s"
//extern char demo_sounds[];

#include <string.h>

// add multiple characters to update buffer
// using horizontal increment
// VBUFSIZE = maximum update buffer bytes
#define VBUFSIZE 128
// index to end of buffer
byte updptr = 0;
// update buffer starts at $100 (stack page)
#define updbuf ((byte*)0x100)

// C versions of macros
#define VRAMBUF_SET(b) updbuf[updptr] = (b);
#define VRAMBUF_ADD(b) VRAMBUF_SET(b); ++updptr

// add EOF marker to buffer (but don't increment pointer)
void vrambuf_end(void) {
  VRAMBUF_SET(NT_UPD_EOF);
}

// clear vram buffer and place EOF marker
void vrambuf_clear(void) {
  updptr = 0;
  vrambuf_end();
}

// wait for next frame, then clear buffer
// this assumes the NMI will call flush_vram_update()
void vrambuf_flush(void) {
  // make sure buffer has EOF marker
  vrambuf_end();
  // wait for next frame to flush update buffer
  // this will also set the scroll registers properly
  ppu_wait_frame();
  // clear the buffer
  vrambuf_clear();
}


void vrambuf_put(word addr, register const char* str, byte len) {
  // if bytes won't fit, wait for vsync and flush buffer
  if (VBUFSIZE-4-len < updptr) {
    vrambuf_flush();
  }
  // add vram address
  VRAMBUF_ADD((addr >> 8) ^ NT_UPD_HORZ);
  VRAMBUF_ADD(addr); // only lower 8 bits
  // add length
  VRAMBUF_ADD(len);
  // add data to buffer
  memcpy(updbuf+updptr, str, len);
  updptr += len;
  // place EOF mark
  vrambuf_end();
}

void clrscr() {
  vrambuf_clear();
  ppu_off();
  vram_adr(0x2000);
  vram_fill(0, 32*28);
  vram_adr(0x0);
  ppu_on_bg();
}

void print( unsigned x, unsigned y, char *str)
{
  vrambuf_put(NTADR_A(x,y), str, strlen(str));
}

void printhex( unsigned y, unsigned x, unsigned size, unsigned val)
{
  char buf[32];
  
  unsigned i;
  for( i=0; i<size; i++ )
  {
    unsigned n=val&0xf;
    val=val>>4;
    
      if( n>9 )
       n=(n-10)+'a';
      else n=n+'0';
      
      buf[(size-1)-i]=n;
  }
  buf[i]=0;
  
  print( x,y, buf );
}

void main(void)
{
  unsigned x=1;
  unsigned p=0; unsigned p2=1;
  
  unsigned ADDR4DUMP=0x8000;
  unsigned ADDR4DUMPt=0x8000;
  
  unsigned ADDR=0x1234;
  unsigned ADDR2=0x0000;
  
  
  unsigned VALW=0x5a;
  unsigned VALR=0x5a;
  unsigned VALW2=0x00;
  unsigned VALR2=0x00;
  
  unsigned RnW=0;
  unsigned RnW2=1;
  
  //
  unsigned hexcol=0;
  unsigned hexrow=0;
  
  pal_col(1,0x04);
  pal_col(2,0x20);
  pal_col(3,0x30);
  
  vrambuf_clear();
  clrscr();
  set_vram_update(updbuf);
  
  // initialize music system
  //famitone_init(after_the_rain_music_data);
  //famitone_init(danger_streets_music_data);
  //sfx_init(demo_sounds);
  print(1,1,"Hex dump tool!");

  // set music callback function for NMI
  //nmi_set_callback(famitone_update);
  // play music
  // music_play(0);

  //enable rendering
  //  ppu_on_all();

  joy_install (joy_static_stddrv);

  // repeat forever
  while(1)
  {
    byte mask=0;
    char pad2=0;
    
    // poll controller 0
    byte joy;
    joy = joy_read (JOY_1);

    //while( joy==0 ) joy = joy_read (JOY_1) ;

    // Move pointer left/right
    if( joy & JOY_LEFT_MASK  ) {  
      if(p==0)p=0; else
      p--;
      mask=JOY_LEFT_MASK;
    }
    
    if( joy & JOY_RIGHT_MASK  ) {  
      if(p==10)p=10; else
      p++;
      mask=JOY_RIGHT_MASK;
    }
    
    
    if( p<=3 ) {
        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (ADDR>>((3-p)*4)) & 0xf;
          if(x==0)x=15; else
          x--;
    	x = x<<((3-p)*4) | (ADDR & (0xffff & (~(0xf<<((3-p)*4)))));
    	ADDR = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (ADDR>>((3-p)*4)) & 0xf;
          if(x>=15)x=0;else
          x++;
    	x = x<<((3-p)*4) | (ADDR & (0xffff & (~(0xf<<((3-p)*4)))));
    	ADDR = x;
          mask=JOY_UP_MASK;
        }
    }
    
    if( p==4 && (joy & JOY_UP_MASK) ) {
    	RnW = 1-RnW;
      	mask=JOY_UP_MASK;
    }
    
    // ---------------------------------------------------
    // Changing Hex Read Address
    if( p>=7 ) {
      	unsigned bitpos=(3-(p-7))*4;

        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (ADDR4DUMPt>>bitpos) & 0xf;
          if(x==0)x=15; else
          x--;
    	  x = x<<bitpos | (ADDR4DUMPt & (0xffff & (~(0xf<<bitpos))));
    	  ADDR4DUMPt = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (ADDR4DUMPt>>bitpos) & 0xf;
          if(x>=15)x=0;else
          x++;
    	  x = x<<bitpos | (ADDR4DUMPt & (0xffff & (~(0xf<<bitpos))));
    	  ADDR4DUMPt = x;
          mask=JOY_UP_MASK;
        }
    }

    // ---------------------------------------------------
    // Changing Hex Read Address
    if( p==5 || p==6 ) {
      	unsigned bitpos=(1-(p-5))*4;

        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (VALW>>bitpos) & 0xf;
          if(x==0)x=15; else
          x--;
    	  x = x<<bitpos | (VALW & (0xffff & (~(0xf<<bitpos))));
    	  VALW = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (VALW>>bitpos) & 0xf;
          if(x>=15)x=0;else
          x++;
    	  x = x<<bitpos | (VALW & (0xffff & (~(0xf<<bitpos))));
    	  VALW = x;
          mask=JOY_UP_MASK;
        }
    }
    
    if( JOY_BTN_1(joy) ) {
      if( p<7 ) {
        if( RnW ) {
          VALR=*((unsigned char*)ADDR) ;
        } else {
          *((unsigned char*)ADDR) = VALW;
        }
      }
      if( p>=7 ) ADDR4DUMP=ADDR4DUMPt;
    }
    
//    ppu_wait_frame();
//    ppu_wait_nmi();
    if(p!=p2) {
      static unsigned oldx=0;
      p2=p;
      
      // Remove old *
      print(oldx,4," ");
      print(oldx,7," ");
      
      if( p<=3 )
        oldx=p+1;
      else if( p==4 )
	oldx=6;
      else if( p==5 || p==6 )
	oldx=(p-5)+8;
      else // Second line
	oldx=(p-6);
      
      if( p<7 )
        print(oldx,4,"*");
      else print(oldx,7,"*");
    }
    
    if(ADDR!=ADDR2) {ADDR2=ADDR;  printhex(5,1,4,ADDR);}

    if( RnW!=RnW2 || VALR!=VALR2 || VALW!=VALW2 ){
      RnW2=RnW;
      VALR2=VALR;
      VALW2=VALW;
      
      if( RnW ) {      
        print(6,5,"R ");
        printhex(5,8,2,VALR);
      } else {
        print(6,5,"W");
        printhex(5,8,2,VALW); 
      }
    }
    printhex(8,1,4,ADDR4DUMPt);
    
    // Would be better to print the hex over successive frames
    vrambuf_flush();    
    {
      static unsigned counter=0;
      counter=counter+1;
      printhex(20,8,4,counter); 
      if( counter>=0x20 ) {
        counter=0;
    
        print(1,9,"     0  1  2  3  4  5  6  7");

        for( hexrow=0; hexrow<8 ; hexrow++ )
        {
          printhex(10+hexrow,1,4,ADDR4DUMP+(hexrow*8));
          for( hexcol=0; hexcol<8 ; hexcol++ )
          {
            unsigned val = *((unsigned*)(ADDR4DUMP+hexcol+(hexrow*8)));
            printhex(10+hexrow,(hexcol*3)+6,2,val);  
          }
          vrambuf_flush();
        }
      }
    }
    //ppu_wait_frame();
    vrambuf_flush();
    
    while(   (joy&mask) ) joy = joy_read (JOY_1) ;

  }
}
