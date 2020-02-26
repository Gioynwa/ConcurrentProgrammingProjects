#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mandelCore.h"

#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;

//h global flag kai to struct gia tis parametrous tis mandel_Calc
/*===================*/
volatile int flag = 0,l = 0;
//flag1 pinakas me 1 an exei teleiosei o ipologismos tou l thread(slice) kai 0 an den exei teleiosei
volatile int *flag1;

struct running {
	mandel_Pars *sections;
	int iterations;
	int *res1;
};

/*===================*/

//h synarthsh metavashs meta th dhmiourgia twn threads 
/*===================*/
void *run_threads(void *thread_info) {
    
        int n;
        
        n = l++; 
	
	struct running *thread_info1;
	
	thread_info1 = (struct running*)malloc(sizeof(struct running));
	
	thread_info1 = (struct running*)thread_info;
	mandel_Calc(thread_info1[0].sections,thread_info1[0].iterations, thread_info1[0].res1);
    
    //prota kanei 1 to slice pou ipologistike apo to l thread kai meta afksanei to metriti gia to epomeno slice pou tha teleiosei ton ipologismo
	flag1[n] = 1;//otan to n-osto thread teleiwsei tote simadeuoyme me 1 ton pinaka twn flags stin thesi n 
    
	
}
/*===================*/

/* basic win management rountines */

static void openDisplay() {
  if (dsp == NULL) { 
    dsp = XOpenDisplay(NULL); 
  } 
}

static void closeDisplay() {
  if (dsp != NULL) { 
    XCloseDisplay(dsp); 
    dsp=NULL;
  }
}

void openWin(const char *title, int width, int height) {
  unsigned long blackC,whiteC;
  XSizeHints sh;
  XEvent evt;
  long evtmsk;

  whiteC = WhitePixel(dsp, DefaultScreen(dsp));
  blackC = BlackPixel(dsp, DefaultScreen(dsp));
  curC = blackC;
 
  win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);

  sh.flags=PSize|PMinSize|PMaxSize;
  sh.width=sh.min_width=sh.max_width=WinW;
  sh.height=sh.min_height=sh.max_height=WinH;
  XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);

  XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
  XMapWindow(dsp, win);
  do {
    XWindowEvent(dsp, win, StructureNotifyMask, &evt);
  } while (evt.type != MapNotify);

  gc = XCreateGC(dsp, win, 0, NULL);

}

void closeWin() {
  XFreeGC(dsp, gc);
  XUnmapWindow(dsp, win);
  XDestroyWindow(dsp, win);
}

void flushDrawOps() {
  XFlush(dsp);
}

void clearWin() {
  XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
  XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
  flushDrawOps();
  XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
  XDrawPoint(dsp, win, gc, x, WinH-y);
  flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
  XEvent evt;

  XSelectInput(dsp, win, ButtonPressMask);
  do {
    XNextEvent(dsp, &evt);
  } while (evt.type != ButtonPress);
  *x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
  XColor clr1,clr2;

  if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
    printf("failed\n"); return;
  }
  XSetForeground(dsp, gc, clr1.pixel);
  curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
  static char cname[128];

  if (v == maxIterations) {
    return("black");
  }
  else {
    sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
    return(cname);
  }
}

int main(int argc, char *argv[]) {
  mandel_Pars pars,*slices;
  int i,j,x,y,nofslices,maxIterations,level,*res;
  int xoff,yoff;
  long double reEnd,imEnd,reCenter,imCenter;
  int temp, number = 0;
  
  //epipleon metavlhtes
  /*===================*/
  pthread_t *worker;
  int check,flag2;
  struct running *thread_info;
  /*===================*/

  printf("\n");
  printf("This program starts by drawing the default Mandelbrot region\n");
  printf("When done, you can click with the mouse on an area of interest\n");
  printf("and the program will automatically zoom around this point\n");
  printf("\n");
  printf("Press enter to continue\n");
  getchar();

  pars.reSteps = WinW; /* never changes */
  pars.imSteps = WinH; /* never changes */
 
  /* default mandelbrot region */

  pars.reBeg = (long double) -2.0;
  reEnd = (long double) 1.0;
  pars.imBeg = (long double) -1.5;
  imEnd = (long double) 1.5;
  pars.reInc = (reEnd - pars.reBeg) / pars.reSteps;
  pars.imInc = (imEnd - pars.imBeg) / pars.imSteps;

  printf("enter max iterations (50): ");
  scanf("%d",&maxIterations);
  printf("enter no of slices: ");
  scanf("%d",&nofslices);
  
  /* adjust slices to divide win height */

  while (WinH % nofslices != 0) { nofslices++;}

  /* allocate slice parameter and result arrays */
  
  slices = (mandel_Pars *) malloc(sizeof(mandel_Pars)*nofslices);
  res = (int *) malloc(sizeof(int)*pars.reSteps*pars.imSteps);
 
  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);
    

  level = 1;
  
  //desmefsi mnimis pinaka gia proto orisma sthn pthread_create
  /*===================*/
  worker = (pthread_t*)malloc(nofslices*sizeof(pthread_t));
  
  //desmefsi pinaka flag1 gia to an kathe thread(slice) exei teleiosei ton ipologismo
  flag1 = (volatile int*)malloc(nofslices*sizeof(volatile int));
  
  /*===================*/

  while (1) {
      
    //arxikopoiisi kathe fora pou ginetai neos ipologismos kai sxediasmos
    l = 0;  
    for(i = 0; i < nofslices; i++) {
      flag1[i] = 0;
    }  
	/*===================*/
	flag = 0;  
    
	/*===================*/

    clearWin();

    mandel_Slice(&pars,nofslices,slices);
    
	/*===================*/
	//dinamikos pinakas thread_info tipou struct gia tin apothikefsi ton parametron tis mandel_Calc
	//gia na anaktithoun apo ta thread workers
	//megethos oso to nofslices
	thread_info = (struct running*)malloc(nofslices*sizeof(struct running));
	
	for (i=0; i<nofslices; i++) {
        //perasma timon orismaton tis mandel_Calc ston dinamiko pinaka thread_info gia kathe slice
		thread_info[i].sections = &slices[i];
		thread_info[i].iterations = maxIterations;
		thread_info[i].res1 = &res[i*slices[i].imSteps*slices[i].reSteps];
		printf("starting slice nr. %d\n",i+1);
        //dimiourgia tou thread pernontas stin routina ti thesi tou pinaka pou antoistoixei se kathe thread(slice)
		check = pthread_create(&worker[i], NULL, run_threads, (void*)&thread_info[i]);
		if(check != 0) {
			printf("thread create failure\n");
			exit(1);
		}
	}
	
	/*===================*/
    
    y = 0;
    number = 0;
    while(flag != nofslices){ //oso den exoun ola ta thread(workers) teleiosei ton ipologismo tous 
    /*===================*/   //i main sxediazei opoios apo tous nofslices workers exei teleiosei  
        while(number != nofslices){ //oso den exoun ola ta thread workers teleiosei ton sxediasmo tous
            flag2 = 0;
            for(i = 0 ; i < nofslices;i++){//elegxos kai anazitisi tou worker pou exei teleiosei
                if(flag1[i] == 1){
                    flag1[i] = 0; //midenizoume gia na min ksanasxediastei to i slice
                    temp = i; //kratame ti thesi tou gia na ton sxediasoume meta
                    number++;
                    flag2 = 1;
                    break;
                }
            }
            if(flag2 == 1) { //otan vrei ton worker pou exei teleiosei vgainei apo to loop gia na ton sxediasei
                break;
            }
        }
        
        
        printf("done\n");
        for (j=0; j<slices[temp].imSteps; j++) {
            for (x=0; x<slices[temp].reSteps; x++) {
                setColor(pickColor(res[y*slices[temp].reSteps+x],maxIterations));
                drawPoint(x,y);
            }
            y++;
        }
        
        flag++; //teleiose o sxediasmos tou temp slice
        
        //mandel_Calc(&slices[i],maxIterations,&res[i*slices[i].imSteps*slices[i].reSteps]);
    }

    
    /* get next focus/zoom point */
    getMouseCoords(&x,&y);
    xoff = x;
    yoff = WinH-y;
    
    /* adjust region and zoom factor  */
    
    reCenter = pars.reBeg + xoff*pars.reInc;
    imCenter = pars.imBeg + yoff*pars.imInc;
    pars.reInc = pars.reInc*ZoomStepFactor;
    pars.imInc = pars.imInc*ZoomStepFactor;
    pars.reBeg = reCenter - (WinW/2)*pars.reInc;
    pars.imBeg = imCenter - (WinH/2)*pars.imInc;
    
    maxIterations = maxIterations*ZoomIterationFactor;
    level++;

  } 
  
  /* never reach this point; for cosmetic reasons */
  free((int*)flag1);
  free(slices);
  free(res);
  free(worker);
  free(thread_info);

  closeWin();
  closeDisplay();

}
