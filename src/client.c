/* 
Rob Reutiman, John Quinn, Patrick Bald
rreutima, jquinn13, pbald
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LINE 4096

/*
 * @func   upload
 * @desc   uploads file to server
 * --
 * @param  s      Socket number
 * @param  fname  File name
 */
void upload(int s, char* fname) {

	// Receive Server Acknowledgement
	int ack;
	if (recv(s, &ack, sizeof(ack), 0) < 0) {
		perror("Error Receiving Server Acknowledgement");
		return;
	}

	// Get File Information
	struct stat fstat;
	if (stat(fname, &fstat) < 0) {
		fprintf(stderr, "Unable to gather file information\n");
		return;
	}

	// Get File Size
	int fsize = fstat.st_size;

	// Send File Size
	if(send(s, &fsize, sizeof(fsize), 0) == -1) {
		perror("Client send error!");
		exit(1);
	}

	// Open File
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "Unable to open file\n");
	}

	// Read Content and Send
	char buff[BUFSIZ];
	while (fsize != 0) {
		// Read Content
		int read = fread(buff, fsize, 1, fp);
		if (read < 0) {
			fprintf(stderr, "Error reading file\n");
			return;
		}
		fsize -= read;

		// Send Content
		if(send(s, buff, read, 0) == -1) {
			perror("Client send error!");
			exit(1);
		}
	}

	// Close File
	fclose(fp);

}

int main(int argc, char * argv[]) {
  /* Variables */
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int SERVER_PORT;

  /* Parse command line arguments */
  if(argc == 3) {
    host = argv[1];
    SERVER_PORT = atoi(argv[2]);
  } else {
    fprintf(stderr, "usage: simplex-talk host\n");
    exit(1);
  }

  /* Translate host name into peer's IP address */
  hp = gethostbyname(host);

  if(!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }

  /* Build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

  /* Create Socket */
  if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket"); 
    exit(1);
  }

  /* Connect to server */
  printf("Connecting to %s on port %d\n", host, SERVER_PORT);

  if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("simplex-talk: connect");
    close(s); 
    exit(1);

  }
  printf("Connection established\n> ");

  /* Client Shell: Send commands to server */
  while(fgets(buf, sizeof(buf), stdin)) {

		// Grab Command, Name, and Length
    char* cmd = strtok(buf, " ");
		char* name  = strtok(NULL, "\t\n\0 ");
		uint16_t len   = strlen(name) - 1;
		fprintf(stdout, "Command: %s; Name: %s; Name Length: %d\n", cmd, name, len);

    /* Send intial operation */
    if(send(s, cmd, strlen(cmd) + 1, 0) == -1) {
      perror("client send error!"); 
      exit(1);
    }

    /* Send command specific data */
    if(!strcmp(cmd, "DN") || !strcmp(cmd, "UP") || !strcmp(cmd, "HEAD") || !strcmp(cmd, "RM") || !strcmp(cmd, "MKDIR") || !strcmp(cmd, "RMDIR") || !strcmp(cmd, "CD")) {

      /* Send length of name */
      u_int16_t l = htons(len);
			fprintf(stdout, "Sending file name length: %d\n", l);
      if(send(s, &l, sizeof(l), 0) == -1) {
        perror("client send error!");
        exit(1);
      }

      /* Send name */
      if(send(s, name, strlen(name), 0) == -1) {
        perror("client send error!"); 
        exit(1);
      }
      
    } else if(!strncmp(cmd, "QUIT", 4)) {
      /* Quit */
      close(s);
      return 0;
    }

    /* Command specific client operations */

    /* UP */
    if(!strcmp(cmd, "UP")) {
			upload(s, name);
    }

    /* DN */
    else if(!strcmp(cmd, "DN")) {

    }

    /* HEAD */
    else if(!strcmp(cmd, "HEAD")) {

    }

    /* RM */
    else if(!strcmp(cmd, "RM")) {

    }

    /* MKDIR */
    else if(!strcmp(cmd, "MKDIR")) {

    }

    /* RMDIR */
    else if(!strcmp(cmd, "RMDIR")) {

    }

    /* CD */
    else if(!strcmp(cmd, "CD")) {

    }


    printf("> ");
  }

  close(s); 
  return 0; 
}
