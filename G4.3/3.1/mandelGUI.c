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

volatile pthread_mutex_t locker_begin, locker_ender, locker, locker_down;
volatile pthread_cond_t cond, cond_down, begin, ender;


//h global flag kai to struct gia tis parametrous tis mandel_Calc
/*===================*/
volatile int flag = 0, l = 0, end = 0, printer, global_counter = 0, val = 0, nof, val_down = 0;
volatile int val_ender = 0, val_begin = 0;
//flag1 pinakas me 1 an exei teleiosei o ipologismos tou l thread(slice) kai 0 an den exei teleiosei
volatile int *flag1;

struct running {
	mandel_Pars *sections;
	int iterations;
	int *res1;
};

volatile struct running *thread_info;

/*===================*/

//h synarthsh metavashs meta th dhmiourgia twn threads 
/*===================*/
void *run_threads() {
    
    int n;
    n = l++;
    
    while(end == 0) {
        
        printf(" thread id : %lu\n",pthread_self());
        
        //ta threads perimenoun ti main na tous eidopoiisei na ksekinisoun ton ipologismo
		/*=========================MONITOR_bEGIN(DOWN)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_begin) != 0){
                perror("");
        }
        
        while(val_begin == 0){
            if (pthread_cond_wait((pthread_cond_t*)&begin, (pthread_mutex_t*)&locker_begin) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
            } 
        }
        val_begin = 0;
        
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker_begin) != 0){
            perror("");
        }
        /*==========================================================================*/
        
		/*=========================MONITOR_bEGIN(UP)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_begin) != 0){
                perror("");
        }
        
        val_begin = 1;
		if (pthread_cond_signal((pthread_cond_t*)&begin) != 0) {                                  
				perror("pthread_cond_signal() error");                                   
		} 
       
		if(pthread_mutex_unlock((pthread_mutex_t*)&locker_begin) != 0){
            perror("");
        }
        /*=======================================================================*/
        
        if(end == 1) {
            printf(" thread_end id : %lu\n",pthread_self());
            break;
        }
        
        struct running *thread_info1;
        thread_info1 = (struct running*)malloc(sizeof(struct running));
        
        thread_info1[0] = thread_info[n];
        printf("working thread id : %lu\n",pthread_self());
        mandel_Calc(thread_info1[0].sections,thread_info1[0].iterations, thread_info1[0].res1);
        
        //prota kanei 1 to slice pou ipologistike apo to l thread kai meta afksanei to metriti gia to epomeno slice pou tha teleiosei ton ipologismo
        flag1[n] = 1;//otan to n-osto thread teleiwsei tote simadeuoyme me 1 ton pinaka twn flags stin thesi n 
        
        printf("flag1[n] = %d , n = %d \n",flag1[n],n);
        
        /*======================*/
        
		/*=========================MONITOR COND_DOWN(DOWN)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_down) != 0){
                perror("");
        }
        
        while(val_down == 0){
            if (pthread_cond_wait((pthread_cond_t*)&cond_down, (pthread_mutex_t*)&locker_down) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
            } 
        }
        val_down = 0;
        
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker_down) != 0){
            perror("");
        }
        /*==============================================================================*/
        
		printer = n;
        
		/*=========================MONITOR COND(UP)===============================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker) != 0){
                perror("");
        }
        
        val = 1;
        if (pthread_cond_signal((pthread_cond_t*)&cond) != 0) {                                        
            perror("pthread_cond_signal() error");
        }
            
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker) != 0){
            perror("");
        }
        /*====================================================================*/
        
        /*======================*/
		/*=========================MONITOR ENdER(DOWN)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_ender) != 0){
                perror("");
        }
        
        while(val_ender == 0){
            if (pthread_cond_wait((pthread_cond_t*)&ender, (pthread_mutex_t*)&locker_ender) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
            } 
        }
        val_ender = 0;
        
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker_ender) != 0){
            perror("");
        }
        /*=========================================================================*/
        
		/*=========================MONITOR ENdER(UP)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_ender) != 0){
                perror("");
        }
        
        val_ender = 1;
		if (pthread_cond_signal((pthread_cond_t*)&ender) != 0) {                                  
				perror("pthread_cond_signal() error");                                   
		} 
       
		if(pthread_mutex_unlock((pthread_mutex_t*)&locker_ender) != 0){
            perror("");
        }
         /*======================================================================*/   
    }
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
  int check,flag2,k;
  int *ycoord;
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
    
  
  nof = nofslices;
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
  
  //pinakas gia apothikefsi ek twn proterwn ton sintetagmenwn tou kathe slice gia tin sxediasi
  ycoord = ( int*)malloc(nofslices*sizeof( int));
  
  /*=========================INITIALIZATIONS=================================*/
  if(pthread_mutex_init((pthread_mutex_t*)&locker_begin, NULL) != 0) {
        perror("");
  }
  
  if(pthread_mutex_init((pthread_mutex_t*)&locker, NULL) != 0) {
        perror("");
  }
   
  if(pthread_mutex_init((pthread_mutex_t*)&locker_down, NULL) != 0) {
        perror("");
  } 
  
  if(pthread_mutex_init((pthread_mutex_t*)&locker_ender, NULL) != 0) {
        perror("");
  }
    
  if(pthread_cond_init((pthread_cond_t*)&cond, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
  }
  
  if(pthread_cond_init((pthread_cond_t*)&cond_down, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
  }
  
  if(pthread_cond_init((pthread_cond_t*)&begin, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
  }
  
  if(pthread_cond_init((pthread_cond_t*)&ender, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
  }

  /*====================================================================*/
  //dinamikos pinakas thread_info tipou struct gia tin apothikefsi ton parametron tis mandel_Calc
  //gia na anaktithoun apo ta thread workers
  //megethos oso to nofslices
  thread_info = (struct running*)malloc(nofslices*sizeof(struct running));
   
  mandel_Slice(&pars,nofslices,slices);
 
  for (i=0; i<nofslices; i++) {
        //perasma timon orismaton tis mandel_Calc ston dinamiko pinaka thread_info gia kathe slice
        thread_info[i].sections = &slices[i];
        thread_info[i].iterations = maxIterations;
        thread_info[i].res1 = &res[i*slices[i].imSteps*slices[i].reSteps];
        printf("starting slice nr. %d\n",i+1);
        check = pthread_create(&worker[i], NULL, run_threads, NULL);
        if(check != 0) {
                printf("thread create failure\n");
                exit(1);
        }
    }
  
	
  while (1) {
      
    //arxikopoiisi kathe fora pou ginetai neos ipologismos kai sxediasmos
     
    for(i = 0; i < nofslices; i++) {
      flag1[i] = 0;
    }  
    
    /*===================*/
    flag = 0;  
    clearWin();
    
    /*===================*/
    
    //theloume na perasoume tis nees parametrous kathe fora sta threads ektos apo tin prwti fora
    //dioti to kaname idi eksw apo tin while 
    if(global_counter != 0) {
        mandel_Slice(&pars,nofslices,slices);
        for (i=0; i<nofslices; i++) {
            //perasma timon orismaton tis mandel_Calc ston dinamiko pinaka thread_info gia kathe slice
            thread_info[i].sections = &slices[i];
            thread_info[i].iterations = maxIterations;
            thread_info[i].res1 = &res[i*slices[i].imSteps*slices[i].reSteps];
            printf("starting slice nr. %d\n",i+1);
        }
    }
	
    /*===================*/
    
    //edw pairnoume tis swstes sintetagmenes ek twn proterwn gia kathe slice p tha sxediastei
    //dld apo poio y tha prepei na ksekinisei
    i = 0;
    for( k = 0;k < nofslices;k++){
        
        ycoord[k] = i;
        i = i + slices[k].imSteps;
    }
    
    val = 0;
    /*===================*/
    //i main eidopoiei ta slices na ksekinisoun ton ipologismo tous 
    
    /*=========================MONITOR_bEGIN(UP)=================================*/
    if(pthread_mutex_lock((pthread_mutex_t*)&locker_begin) != 0){
		perror("");
	}
        
	val_begin = 1;
	if (pthread_cond_signal((pthread_cond_t*)&begin) != 0) {                                  
		perror("pthread_cond_signal() error");                                   
	} 
       
	if(pthread_mutex_unlock((pthread_mutex_t*)&locker_begin) != 0){
		perror("");
	}
    /*=======================================================================*/
    
	/*===================*/
    val = 0;
    val_down = 0;
    number = 0;
    
    while(flag != nofslices){ //oso den exoun ola ta thread(workers) teleiosei ton ipologismo tous 
    /*===================*/   //i main sxediazei opoios apo tous nofslices workers exei teleiosei 
    
        //i main ksipnaei ton worker p exei teleiwsei gia na sxediasei to slice tou
        
        /*=============================*/
        
        /*=========================MONITOR COND_DOWN(UP)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker_down) != 0){
                perror("");
        }
        
        val_down = 1;
        if (pthread_cond_signal((pthread_cond_t*)&cond_down) != 0) {                                        
            perror("pthread_cond_signal() error");
        }
            
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker_down) != 0){
            perror("");
        }
        /*===========================================================================*/
        
        /*=========================MONITOR COND(DOWN)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&locker) != 0){
                perror("");
        }
        
        while(val == 0){
            if (pthread_cond_wait((pthread_cond_t*)&cond, (pthread_mutex_t*)&locker) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
            } 
        }
        val = 0;
        
        if(pthread_mutex_unlock((pthread_mutex_t*)&locker) != 0){
            perror("");
        }
        /*=========================================================================*/
        
        printf("done\n");
        for (j=0; j<slices[printer].imSteps; j++) {
            for (x=0; x<slices[printer].reSteps; x++) {
                setColor(pickColor(res[ycoord[printer]*slices[printer].reSteps+x],maxIterations));
                drawPoint(x,ycoord[printer]);
            }
            ycoord[printer]++;
        }
        
        flag++; //teleiose o sxediasmos tou temp slice
        
        /*===========================*/
    }
    
    //eidopoiei i main tous workers outws wste na pane stin arxi, afou idi exoun teleiwsei oloi 
    
    /*=========================MONITOR ENdER(UP)=================================*/
    if(pthread_mutex_lock((pthread_mutex_t*)&locker_ender) != 0){
		perror("");
	}
        
	val_ender = 1;
	if (pthread_cond_signal((pthread_cond_t*)&ender) != 0) {                                  
		perror("pthread_cond_signal() error");                                   
	} 
       
	if(pthread_mutex_unlock((pthread_mutex_t*)&locker_ender) != 0){
		perror("");
	}
	/*=======================================================================*/
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
	global_counter++;
	
  } 
  end = 1;
  
  
  /* never reach this point; for cosmetic reasons */
  free((int*)flag1);
  free(slices);
  free(res);
  free(worker);
  free((struct running*)thread_info);

  closeWin();
  closeDisplay();

}
