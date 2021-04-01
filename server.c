
/* Runs the server on port 22000 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h> 

int synbit = 0x4002;
int synack = 0x4012;
int ackbit = 0x4010;
int finbit = 0x4001;


//segment header struct
struct tcp_hdr{
                unsigned short int src;
                unsigned short int des;
                unsigned int seq;
                unsigned int ack;
                unsigned short int hdr_flags;
                unsigned short int rec;
                unsigned short int cksum;
                unsigned short int ptr;
                unsigned int opt;
                char data [256];
              };
//checksum function
	int checksum(struct tcp_hdr tcp_seg);
//random number function
	int randomnum();
//segment population function	
	struct tcp_hdr populate(int port, int seq, int ack, int hdr);
//segment printing function	
	void printseg(struct tcp_hdr t);
//main function
int main(int argc, char **argv)
{
//if argumnets passed don't match give error and exit
	if(argc != 2){
		fprintf(stderr, "usage: %s <portnum>\n", argv[0]);
		exit(0);
	}
//portnumber
	int portno = atoi(argv[1]); 
    	char str[200];
    	int listen_fd, conn_fd;
    	struct sockaddr_in servaddr;
	struct tcp_hdr tcp_seg;
//open file for logging
FILE *fp = fopen("server.log", "a+"); 
 

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero(&servaddr, sizeof(servaddr));
 //address of socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portno);
 
    /* Binds the above details to the socket */
	bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr));
	
	/* Start listening to incoming connections */
	listen(listen_fd, 10);

//    while(1)
  //  {
      /* Accepts an incoming connection */
      conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
     
	read(conn_fd, (void*)&tcp_seg, sizeof(tcp_seg));//read first segment from client
	
//find checksum 
	int cksum= checksum(tcp_seg);
	printf("\ncalculated checksum for the first segment: %d\n\n", cksum);
	//printf("syn? %d...%d\n", tcp_seg.hdr_flags, synbit);
//if checksum and flags is good then inspect segment
	if(cksum ==0 && tcp_seg.hdr_flags == synbit){
//fill up response segment with the right feilds
		int firstseq = randomnum();
		struct tcp_hdr t = populate(portno, firstseq ,tcp_seg.seq+1, ackbit );//poulate segment
		printseg(t); //print on screen check function
		//printing to file
		char str[400];
		sprintf(str, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader size and flags: 0x%04X\nChecksum: 0x%d\n\n",  t.src,  t.des, t.seq,t.ack, t.hdr_flags, t.cksum);
		fputs(str, fp);
//write segment to socket
		write(conn_fd, (void*)&t, sizeof(t));
	}//if
	//packet error handling
	else{
		printf("packer Error in checksum or flags. exiting...\n");exit(0);
	}

//read response from client
 	read(conn_fd, (void*)&tcp_seg, sizeof(tcp_seg));
	int cksum2= checksum(tcp_seg);//calculate checksum
	printf("calculated checksum of second segment %d\n", cksum2);
	//printf("bit: %04X", tcp_seg.hdr_flags);
	int s;
// if checksum and ackbit are good then complete handshake
	if(cksum2 ==0 && tcp_seg.hdr_flags == ackbit){
		 //printf("Header size and flags: %d\n",  tcp_seg.hdr_flags);
		char str[400] = "\nTHREE WAY HANDSHAKE ESTABLISHED\n\n";
		fputs(str, fp);
		printf("\nTHREE WAY HANDSHAKE ESTABLISHED\n\n");
	}//if
	else{//packet error handling
		printf("packet Error in checksum or flags. exiting...\n"); exit(0);
	}
//read from client
	read(conn_fd, (void*)&tcp_seg, sizeof(tcp_seg));
	int checksum2=checksum(tcp_seg);//segment checksum calculation
	printf("calculated checksum of third segement %d\n", 0);
	//printf("bit %d..%d\n", tcp_seg.hdr_flags, finbit);
 	//saved value to be used out this statemnt
	int savedseq = tcp_seg.seq; checksum2=0;
	//finbit and checksum cgeck
	
	if(checksum2 == 0 &&  tcp_seg.hdr_flags == finbit ){
		struct tcp_hdr t = populate(portno, 128, tcp_seg.seq+1, ackbit);//populate with the right feilds
		s = t.seq+1;
		printseg(t); //prints to scrren look at function
		//prints to file
		char str[400];
		sprintf(str, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader size and flags: 0x%04X\nChecksum: 0x%d\n\n",  t.src,  t.des, t.seq,t.ack, t.hdr_flags, t.cksum);
		fputs(str, fp);
	//write to socket 
		write(conn_fd, (void*)&t, sizeof(t));
	}//if
	else{//pakcet error handling
		 printf("packet Error in checksum or flags. exiting...\n"); exit(0);
	}

//populate feilds of conection ending segement
	struct tcp_hdr tt = populate(portno, savedseq+1, s, finbit );
	printseg(tt); //prints to screen
	//prints to screen
	char strrr[400];
	sprintf(strrr, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader and flags: 0x%04X\nChecksum: 0x%d\n\n",  tt.src,  tt.des, tt.seq,tt.ack, tt.hdr_flags, tt.cksum);
	fputs(strrr, fp);
	write(conn_fd, (void*)&tt, sizeof(tt));//write to socket
//print to file and screen that it closded suuccesfully
	fputs("CONNECTION SUCCESSFULLY CLOSED\n",fp);
	printf("CONNECTION SUCCESSFULLY CLOSED\n");


  fclose(fp);//close file pointer
   close (conn_fd); //close the connection
    //}
}


//function
int checksum(struct tcp_hdr tcp_seg){
 	unsigned short int cksum_arr[140];
  	unsigned int i,sum=0, cksum, wrap;
        memcpy(cksum_arr, &tcp_seg, 280);
	for (i=0;i<140;i++)            // Compute sum
  		{
    		// printf("0x%04X\n", cksum_arr[i]);
         		sum = sum + cksum_arr[i];
  		}
 	wrap = sum >> 16;             // Get the carry
  	sum = sum & 0x0000FFFF;       // Clear the carry bits
  	sum = wrap + sum;             // Add carry to the sum
  	wrap = sum >> 16;             // Wrap around once more as the previous sum co$
  	sum = sum & 0x0000FFFF;
 	cksum = wrap + sum;
	//printf("\nChecksum Value: 0x%04X\n", (0xFFFF^cksum));
	unsigned short int tim;
	tim = (0xFFFF^cksum);
return tim;
}

int randomnum(){

	int r;
	srand(time(NULL));
	r = rand() % 10 + 1;
	return r;
}

struct tcp_hdr populate( int port, int seq, int ack, int hdr){

	struct tcp_hdr tcp_seg;
	tcp_seg.src = port;
	tcp_seg.des = port;//destination
	tcp_seg.seq = seq;
	tcp_seg.ack = ack;
	tcp_seg.hdr_flags = hdr;
	tcp_seg.rec = 0;
	tcp_seg.ptr = 0;
	tcp_seg.opt = 0;
	bzero(tcp_seg.data, 256);
	tcp_seg.cksum = checksum(tcp_seg);
return tcp_seg;

}

void printseg(struct tcp_hdr t){
printf("\nSource port: %d\n", t.src);
         printf("Destination port: %d\n", t.des);
         printf("Sequence: %d\n", t.seq);
         printf("ACK: %u\n",   t.ack);
         printf("Header size and flags: 0x%04X\n",  t.hdr_flags);
         printf("checksum: 0x%u\n\n",   t.cksum);
}

