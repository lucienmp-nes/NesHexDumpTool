
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
#include <string.h>

// VRAM graphics
#include "vrambuf.h"
//#link "vrambuf.c"

// CC65 : https://github.com/cc65/cc65/blob/master/include/joystick.h
#include <joystick.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"


// Clear display, enable vram setup
void vrambuf_clrscr() {
  vrambuf_clear();
  ppu_off();
  vram_adr(0x2000);
  vram_fill(0, 32*28);
  vram_adr(0x0);
  ppu_on_bg();
}

void print( unsigned y, unsigned x, char *str)
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
  
  print( y, x, buf );
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
  
  // Color def for fonts
  pal_col(1,0x04);
  pal_col(2,0x20);
  pal_col(3,0x30);
  
  // Clear and initialize
  vrambuf_clear();
  vrambuf_clrscr();
  set_vram_update(updbuf);

  // Static screen information
  print(1,1,"Hex dump tool!");
  print(9,1,"     0  1  2  3  4  5  6  7");

  //enable rendering
  //  ppu_on_all();

  // CC65 Joystick generalization
  joy_install (joy_static_stddrv);

  // repeat forever
  while(1)
  {
    byte mask=0;
    char pad2=0;
    
    // poll controller 0
    byte joy;
    joy = joy_read (JOY_1);

    // ---------------------------------------------------
    // Handle Left/Right joystick

    // Move "*" pointer left/right
    if( joy & JOY_LEFT_MASK  ) {  
      if(p==0) p=0; else p--;
      mask=JOY_LEFT_MASK;
    }
    
    if( joy & JOY_RIGHT_MASK  ) {  
      if(p==10) p=10; else p++;
      mask=JOY_RIGHT_MASK;
    }
    
    // ---------------------------------------------------
    // Changing Write/Read Address, press A button to read/write to it
    if( p<=3 ) {
        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (ADDR>>((3-p)*4)) & 0xf;
          if(x==0) x=15; else x--;
    	  x = x<<((3-p)*4) | (ADDR & (0xffff & (~(0xf<<((3-p)*4)))));
    	  ADDR = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (ADDR>>((3-p)*4)) & 0xf;
          if(x>=15) x=0;else x++;
    	  x = x<<((3-p)*4) | (ADDR & (0xffff & (~(0xf<<((3-p)*4)))));
    	  ADDR = x;
          mask=JOY_UP_MASK;
        }
    }
    
    // ---------------------------------------------------
    // Changing R<->W
    if( p==4 && (joy & JOY_UP_MASK) ) {
    	RnW = 1-RnW;
      	mask=JOY_UP_MASK;
    }
    
    // ---------------------------------------------------
    // Changing Write value
    if( p==5 || p==6 ) {
      	unsigned bitpos=(1-(p-5))*4;

        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (VALW>>bitpos) & 0xf;
          if(x==0) x=15; else x--;
    	  x = x<<bitpos | (VALW & (0xffff & (~(0xf<<bitpos))));
    	  VALW = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (VALW>>bitpos) & 0xf;
          if(x>=15) x=0;else x++;
    	  x = x<<bitpos | (VALW & (0xffff & (~(0xf<<bitpos))));
    	  VALW = x;
          mask=JOY_UP_MASK;
        }
    }
    
    // ---------------------------------------------------
    // Changing Hex Dump Read Address, press A button to lock in
    if( p>=7 ) {
      	unsigned bitpos=(3-(p-7))*4;

        // Counter digit UP/DOWN
        if( joy & JOY_DOWN_MASK  ) {  
          x = (ADDR4DUMPt>>bitpos) & 0xf;
          if(x==0) x=15; else x--;
    	  x = x<<bitpos | (ADDR4DUMPt & (0xffff & (~(0xf<<bitpos))));
    	  ADDR4DUMPt = x;
          mask=JOY_DOWN_MASK;
        }

        if( joy & JOY_UP_MASK  ) {
          x = (ADDR4DUMPt>>bitpos) & 0xf;
          if(x>=15) x=0;else x++;
    	  x = x<<bitpos | (ADDR4DUMPt & (0xffff & (~(0xf<<bitpos))));
    	  ADDR4DUMPt = x;
          mask=JOY_UP_MASK;
        }
    }

    // ---------------------------------------------------
    // Action the hex r/w, or hex dump address
    if( JOY_BTN_1(joy) ) {
      // Read/Write data to selected address
      if( p<7 ) {
        if( RnW ) {
          VALR=*((unsigned char*)ADDR) ;
        } else {
          *((unsigned char*)ADDR) = VALW;
        }
      }
      
      // Lock in hex dump address
      if( p>=7 ) ADDR4DUMP=ADDR4DUMPt;
    }

    /* Display Layout
     *
     *    1: Hex dump tool!
     *    2: 
     *    3: *
     *    4: 1234 W 5a     OR 1234 R ??
     *    5:
     *    6:
     *    7: 8000
     *    8:       0   1 ...
     *    9: 8000  xx yy
     */
//    ppu_wait_frame();
//    ppu_wait_nmi();
    if(p!=p2) {
      static unsigned oldx=0;
      p2=p;
      
      // Remove old *
      print(4,oldx," ");
      print(7,oldx," ");
      
      if( p<=3 )
        oldx=p+1;
      else if( p==4 )
	oldx=6;
      else if( p==5 || p==6 )
	oldx=(p-5)+8;
      else // Second line
	oldx=(p-6);
      
      if( p<7 )
        print(4,oldx,"*");
      else print(7,oldx,"*");
    }
    
    if(ADDR!=ADDR2) {ADDR2=ADDR;  printhex(5,1,4,ADDR);}

    if( RnW!=RnW2 || VALR!=VALR2 || VALW!=VALW2 ){
      RnW2=RnW;
      VALR2=VALR;
      VALW2=VALW;
      
      if( RnW ) {      
        print(5,6,"R ");
        printhex(5,8,2,VALR);
      } else {
        print(5,6,"W");
        printhex(5,8,2,VALW); 
      }
    }
    printhex(8,1,4,ADDR4DUMPt);
    
    // Would be better to print the hex over successive frames
    vrambuf_flush();
    
    // After a few frames dump the RAM, keep dumping incase something 
    // changes.
    {
      static unsigned counter=0;
      counter=counter+1;
      printhex(20,8,4,counter); 
      if( counter>=0x20 ) {
        counter=0;
    
        for( hexrow=0; hexrow<8 ; hexrow++ )
        {
          printhex(10+hexrow,1,4,ADDR4DUMP+(hexrow*8));
          for( hexcol=0; hexcol<8 ; hexcol++ )
          {
            unsigned val = *((unsigned char*)(ADDR4DUMP+hexcol+(hexrow*8)));
            printhex(10+hexrow,(hexcol*3)+6,2,val);  
          }
          vrambuf_flush();
        }
      }
    }
    
    // Flush whatevers left, just lazy.
    //ppu_wait_frame();
    vrambuf_flush();

    // Make sure the joystick changed away from whats being pressed now
    while( (joy&mask) ) joy = joy_read (JOY_1) ;
  } // end while(1);
}
