#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

pthread_mutex_t mutex; 
pthread_t thr, thr2, thr3;
int my_port = 0, peer_port = 0, sckt = -1;
int running = 1; 

struct sockaddr_in peer_addr;
socklen_t len = sizeof(peer_addr); 

char msg[101] = {0}; 

void print_newline(){
	printf(">>> ");
	fflush(stdout); 
}

void *rcv_msg(){
	char buf[101] = {0};

	while(running) {
		if(recvfrom(sckt, buf, sizeof(buf), MSG_WAITALL, (struct sockaddr *)&peer_addr, &len) == -1) {
		    perror("Error: Recvfrom failed.");
	} else {
		if(peer_port == 0) {
			peer_port = peer_addr.sin_port;
		}
		printf("[%d]: %s\n", peer_port, buf);
	   	print_newline();	
	}	
	}
}

void *send_msg() {
	while(running) {
		if(msg[0] != '\0' && peer_port != 0) {
			if(sendto(sckt, msg, strlen(msg) + 1, MSG_CONFIRM, (const struct sockaddr *) &peer_addr, len) == -1){
				perror("Error: Sendto failed.");
			}
			pthread_mutex_lock(&mutex);
			memset(msg, 0, sizeof(msg));  
     		pthread_mutex_unlock(&mutex);
		}
	}
}

void *get_string() {
	while(running) {
		print_newline();

		char input[102]; 
		if(fgets(input, sizeof(input), stdin) == NULL) {
			perror("Error reading from stdin");
			continue; 	
		}

		char *end = strchr(input, '\n'); 
		if(end != NULL) {
			*end = '\0';
		    pthread_mutex_lock(&mutex);
			strcpy(msg, input); 	
			pthread_mutex_unlock(&mutex);
		} else {
			printf("Message is too long. Maximum message length is 100 chars.\n");
			continue; 
		}
	}
}

void print_usage() {
	printf("Usage: ./chat <port> [peer-port]\n"); 
}

int check_port(int port){
	if(port >= 1024 && port <= 65535) {
		return 1; 
	}

	printf("Enter a port between 1024 and 65535.\n"); 
	return -1; 
}

static void sigpipe_handler(int signum) {
	running = 0; 
	printf("\n");

	pthread_cancel(thr);
	pthread_join(thr, NULL);

	pthread_cancel(thr2);
	pthread_join(thr2, NULL);	

	pthread_cancel(thr3);
    pthread_join(thr3, NULL);

	pthread_mutex_destroy(&mutex);

	if(sckt != -1) {
		close(sckt);
	}

	exit(signum);
}

int main(int argc, char **argv) {
	pthread_mutex_init(&mutex, NULL);
	signal(SIGINT, sigpipe_handler);
	
	if(argc == 1) {
		print_usage();
		return EXIT_FAILURE; 
	}

	my_port = atoi(argv[1]); 
	if(check_port(my_port) < 0) {
		return EXIT_FAILURE; 
	}

	peer_port = 0; 
	if(argv[2] != NULL) {
		peer_port = atoi(argv[2]);
		if(check_port(peer_port) < 0) {
			return EXIT_FAILURE; 
		}
	}

	if((sckt = (socket(AF_INET, SOCK_DGRAM, 0))) == -1) {
		perror("Error creating socket\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in my_addr; 
	memset(&my_addr, 0, sizeof(my_addr)); 
	memset(&peer_addr, 0, sizeof(peer_addr)); 

	my_addr.sin_family = AF_INET;
    my_addr.sin_port = my_port; 
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(peer_port != 0) {
		peer_addr.sin_family = AF_INET;
    	peer_addr.sin_port = peer_port; 
    	peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   	
	}	
	
	if(bind(sckt, (const struct sockaddr *) &my_addr, sizeof(my_addr)) == -1) {
			perror("Error binding address to socket");
			return EXIT_FAILURE;
	}

	if(pthread_create(&thr, NULL, rcv_msg, NULL) < 0) {
		perror("Error creating thread");
		return EXIT_FAILURE;
	};

	if(pthread_create(&thr2, NULL, send_msg, NULL) < 0) {
		perror("Error creating thread");
		return EXIT_FAILURE;
	};

 	if(pthread_create(&thr3, NULL, get_string, NULL) < 0) {
        perror("Error creating thread");
        return EXIT_FAILURE;
    };

	while(running) {}

	return EXIT_SUCCESS; 
}
