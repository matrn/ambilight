/*
Data structure for UDP leds protocol:

start char,start LED,stop LED,R,G,B,stop char
1,0-255,0-255,0-255,0-255,0-255,0x03

+------------+-----------+----------+-------+-------+-------+-----------+
| start char | start LED | stop LED |   R   |   G   |   B   | stop char |
+------------+-----------+----------+-------+-------+-------+-----------+
| 1		     | 0-255	 | 0-255	| 0-255 | 0-255 | 0-255 | 0x03	    |
+------------+-----------+----------+-------+-------+-------+-----------+

start chars:
  0 - 1. WS2812 - without response
  1 - 1. WS2812 - with response
  
  2 - 2. WS2812 - without response
  3 - 2. WS2812 - with response

  8 - bulbs - without response
  9 - bulbs - with response

this table generator site: https://ozh.github.io/ascii-tables/
*/
/*
 * Linux install: sudo apt-get install libx11-dev
 * Linux compile: gcc ambilight.c -o ambilight -lX11
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



#define LEDS 82   /* number of leds */
#define Xpix 20   /* number of pixels in X plane */
#define Ypix 20   /* number of pixels in Y plane */
#define SLEEP_TIME 30000   /* time delay between getting pixels - defalt 50 000*/

#define TEST_LEDS_DELAY 200000   /* time between colors test */

typedef unsigned char byte;   /* define byte data type */
typedef char sbyte;   /* define byte data type */


#define MAX_SEND_TRY 5   /* maximum tries of data send */

const int port = 7879;   /* port of server */
const char host[] = "192.168.0.111";   /* IP of server */


/* sock declarations */
int sockfd;
struct sockaddr_in serv,client;
socklen_t l,m;


void set_leds(char type, byte start_led, byte stop_led, byte r, byte g, byte b);   /* send leds values without server response */
byte set_leds_with_response(char type, byte start_led, byte stop_led, byte r, byte g, byte b);   /* send leds values with server response */

byte VERBOSE = 0;



int main(int argc, char ** argv){   
	int RGBdata[3];   //last R G B values
	int Rval = 0;   /* current R value */
	int Gval = 0;   /* current G value */
	int Bval = 0;   /* current B value */
	
	int xPlus, yPlus;   /* size of X and Y increment */
	int width, height;   /* width and height of display */

	int x = 0;   /* X position in for loop */
	int y = 0;   /* Y position in for loop */

	int pixels = 0;   /* number of analyzed pixels */

	char host2[32];
	unsigned char host2_enable = 0;

	/* -----X11 variables----- */
	Display* d;   /* X display */
	Screen*  s;   /* X screen */
	Window rootWindow;   /* X root window */

	//XColor c;   /* X color */
	XImage *image;   /* X image */
	/* -----X1 variables----- */


	if(argc > 1 && strcmp(argv[1], "-v") == 0){
		puts("Verbose mode enabled");
		VERBOSE = 1;
	}

	if(argc > 2){
		if(strcmp(argv[1], "-i") == 0){
			strcpy(host2, argv[2]);
			host2_enable = 1;
		}

		if(strcmp(argv[2], "-i") == 0){
			strcpy(host2, argv[3]);
			host2_enable = 1;
		}
	}


	/* -----sock settings----- */
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);

	if(host2_enable){
		serv.sin_addr.s_addr = inet_addr(host2);
	}else{
		serv.sin_addr.s_addr = inet_addr(host);
	}

	l = sizeof(client);
	m = sizeof(serv);  
	/* -----sock settings----- */

	/* ----------LED SERVER CONNECTION TEST---------- */
	puts("Testing connection: ");
	set_leds('0', 0, LEDS - 1, 0, 0, 0);   /* send leds data without server response */
	set_leds('0', 0, LEDS - 1, 255, 0, 0);   /* send leds data without server response */
	usleep(TEST_LEDS_DELAY);
	set_leds('0', 0, LEDS - 1, 0, 0, 0);   /* send leds data without server response */
	set_leds('0', 0, LEDS - 1, 0, 255, 0);   /* send leds data without server response */
	usleep(TEST_LEDS_DELAY);
	set_leds('0', 0, LEDS - 1, 0, 0, 0);   /* send leds data without server response */
	set_leds('0', 0, LEDS - 1, 0, 0, 255);   /* send leds data without server response */

	usleep(TEST_LEDS_DELAY);
	
	/* send leds data with response */
	if(set_leds_with_response('1', 0, LEDS -1, 0, 0, 0) == 0){
		puts("All is OK");
	}else{
		puts("ERROR while sending!");
		puts("Close this program with CTRL+C");
	}
	/* ----------LED SERVER CONNECTION TEST---------- */

	/* -----X11----- */
	d = XOpenDisplay(NULL);
	s = DefaultScreenOfDisplay(d); 
	rootWindow = RootWindow(d, DefaultScreen(d));

	width  = s->width;
	height = s->height;

	printf("Display width: %d and height: %d\n", width, height);
	/* -----X11----- */
	

	xPlus = (float)width/Xpix + 0.5;
	yPlus = (float)height/Ypix + 0.5;

	//printf("Xplus = %d, Yplus = %d \n", xPlus, yPlus);
	puts("Running");

	while(1){   /* never ending loop */
		/* null all varaibles */
  		Rval = 0;
   		Gval = 0;
		Bval = 0;

		x = 0;
		y = 0;

		pixels = 0;
		
		for(y = 0; y < height; y += yPlus){					 
			if(y < height){	
				//printf("X = %d   Y = %d\n", x, y);
				image = XGetImage(d, rootWindow, 0, y, width, 1, AllPlanes, ZPixmap);   /* get display image */
				//puts("HERE");
				for(x = 0; x < width; x += xPlus){ 
					if(x < width){
						/*
						c.pixel = XGetPixel(image, x, 0);   						
						XQueryColor(d, DefaultColormap(d, DefaultScreen (d)), &c);

						Rval += c.red/256;
						Gval += c.green/256;
						Bval += c.blue/256;
						*/
						unsigned long pixel = XGetPixel(image, x, 0);
						//printf("%lu\n", pixel);
						//place pixel in rgb array
						Rval += (pixel >> 16) & 0xff;
						Gval += (pixel >>  8) & 0xff;
						Bval += (pixel >>  0) & 0xff;


						pixels ++;
					}

				}
				XDestroyImage(image);
				//XFree(image);

			}	 
		}

		
		Rval /= pixels;
		Gval /= pixels;
		Bval /= pixels;

		if(VERBOSE) printf("Got: %d pixels\n", pixels);

		if(Rval != RGBdata[0] || Gval != RGBdata[1] || Bval != RGBdata[2]){
			RGBdata[0] = Rval;
			RGBdata[1] = Gval;
			RGBdata[2] = Bval;

			if(VERBOSE) printf("%d %d %d \n", Rval, Gval, Bval);

			set_leds('0', 0, LEDS - 1, Rval, Gval, Bval);   /* send leds values */
		}else{
			if(VERBOSE) puts("NO change, no send");
		}
		usleep(SLEEP_TIME);
	}
	
	return 0;
}





void set_leds(char type, byte start_led, byte stop_led, byte r, byte g, byte b){
	char data[7];

	
	data[0] = type;
	data[1] = start_led;
	data[2] = stop_led;
	data[3] = r;
	data[4] = g;
	data[5] = b;
	data[6] = 3;

	sendto(sockfd, data, 7, 0, (struct sockaddr *)&serv, m);   /* send data */
}



byte set_leds_with_response(char type, byte start_led, byte stop_led, byte r, byte g, byte b){
	char data[7];   /* data for send */ 
	char recv[8];   /* received data*/
	sbyte recv_len = 0;   /* length of received data */
	int a = 0;   /* variable for everything */
	
	data[0] = type;
	data[1] = start_led;
	data[2] = stop_led;
	data[3] = r;
	data[4] = g;
	data[5] = b;
	data[6] = 3;

	sendto(sockfd, data, 7, 0, (struct sockaddr *)&serv, m);   /* send data */


	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;   /* 100 ms */

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){   /* set socket timeout */
		perror("Set timeout error");
	}

	for(a = 0; a < MAX_SEND_TRY; a ++){
		recv_len = recvfrom(sockfd, recv, sizeof(recv), 0,(struct sockaddr *)&client, &l);
		//printf("RECV LEN: %d\n", recv_len);
		
		if(recv_len == 0 || recv_len > (sbyte)sizeof(recv)){   /* timeout */
			puts("Timeout, resending");
			sendto(sockfd, data, 7, 0, (struct sockaddr *)&serv, m);   /* resend data */
		}
		else if(recv_len == -1){
			perror("Receiving response error");
		}
		else if(recv_len >= 3){
			/*for( int q = 0; q < recv_len; q ++){
				printf("C%d=%d, ", q, recv[q]);
			}
			puts("");*/

			if(recv[0] == 2 && recv[1] == 7 && recv[2] == 3){
				return 0;   /* all is OK */
			}
			else{
				puts("Wrong data, resending");
				sendto(sockfd, data, 7, 0, (struct sockaddr *)&serv, m);   /* resend data */
			}
		}
		else{
			puts("Wrong length, resending");
			sendto(sockfd, data, 7, 0, (struct sockaddr *)&serv, m);   /* resend data */
		}
	}

	return -1;   /* return error */
}