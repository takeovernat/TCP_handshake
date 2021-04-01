/* Client Code */
/* Connect to port 22000 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include  <time.h>


int synbit = 0x4002;
int synack = 0x4012;
int ackbit = 0x4010;
int finbit = 0x4001;

int randomnum();

//structure used for segments 
struct tcp_hdr{
                unsigned short int src; //sourcr
                unsigned short int des; //detination
                unsigned int seq; //sequence
                unsigned int ack; //acknowledgment
                unsigned short int hdr_flags;//flags and header
                unsigned short int rec; //recieve window
                unsigned short int cksum; //checksum
                unsigned short int ptr; //urgent pointer
                unsigned int opt; //option feild
                char data [256]; //data
              };
//checksum function declaration
int checksum(struct tcp_hdr tcp_seg); 
//to populate segment
struct tcp_hdr populate(int port,int seq, int ack, int hdr);
//to print segment
void printseg(struct tcp_hdr t);
//main function
int main (int argc, char **argv)
{
  //socket descriptor
    int sockfd, n;
    int len = sizeof(struct sockaddr);
    
    struct sockaddr_in servaddr; //socket address struct    
    struct tcp_hdr tcp_seg;
  unsigned int i,sum=0, cksum, wrap;//for checksum use
	//if port num not given as arg exit
 	if(argc != 2){
	fprintf(stderr, "usage: %s <portnum>\n", argv[0]);
	exit(0);
	}
//file pointer for log file
	FILE *fp;
       fp = fopen("client.log", "a");


    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
   int portno = atoi(argv[1]);//port number 
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno); // Server port number

    /* Convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET,"129.120.151.95",&(servaddr.sin_addr));

    /* Connect to the server */
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

//random sequence number
	int firstseq = randomnum();
//populate the segment check out the function below
	tcp_seg = populate(portno, firstseq, 0, synbit);

//begginig handshake
	printf("\nHANDSHAKE SEGMENTS\n");
//print segment on screen
	printseg(tcp_seg); 
//print to log file
	char str[400];int bk=0;
	sprintf(str, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader size and flags: 0x%04X\nChecksum: 0x%d\n\n",  tcp_seg.src,  tcp_seg.des, tcp_seg.seq,  tcp_seg.ack, tcp_seg.hdr_flags, tcp_seg.cksum);	
	fputs(str, fp); //put it in log file after putting together
 //write the segment to server
 if( write(sockfd, ((const void *)&tcp_seg), sizeof(tcp_seg)) < 0 ){printf("error\n");exit(0);}
	//read the respose  
  read(sockfd, (void*)&tcp_seg, sizeof(tcp_seg));
//calculate checksum
unsigned int cksum2 = checksum(tcp_seg);
printf("calculated checksum for the segment recieved: %d\n", cksum2 );
	//printf("flag: %d\n", tcp_seg.hdr_flags);
	//printf("flag: %d\n", ackbit);
//checks to see if calculated checksum and SYNACK bit are as expected
if(cksum2==0 && ackbit == tcp_seg.hdr_flags){
//fill struct with neccessary response
	struct tcp_hdr k = populate(portno, tcp_seg.ack, tcp_seg.seq+1, ackbit);
	printseg(k);
//print to screen
	char str[400];
	sprintf(str, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader size and flags: 0x%04X\nChecksum: 0x%d\n\n", k.src,k.des,k.seq, k.ack, k.hdr_flags, k.cksum);
	fputs(str, fp);//write to file also
//write response to server
	write(sockfd, (const void *)&k, sizeof(k));

	}//if
	else{//handle packet error
		printf("Packet Error in checksum or flags exiting...\n");exit(0);
	}//if
//closing segment starts here
	printf("CLOSING SEGMENTS\n\n");
//the first closing statement to be filed with
//the approperate information
	struct tcp_hdr kk = populate(portno, 256, 128, finbit);
//print to screen
	printseg(kk);
	int savedseq = kk.seq;
	char strrr[400];
	sprintf(strrr, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeadersize and flags: 0x%04X\nChecksum: 0x%d\n\n", kk.src,kk.des,kk.seq, kk.ack, kk.hdr_flags, kk.cksum);
	fputs(strrr, fp); //also put in log file
	write(sockfd, (const void *)&kk, sizeof(kk));//and write to socket to server
//read response from server 
	struct tcp_hdr kt, kf;
	read(sockfd, (void *)&kf, sizeof(kf));
//	printf("\n\nflag: %d..%d..%d\n", kt.hdr_flags, ackbit, finbit);
	 int ck = checksum(kf);
	printf("second checksum: %d\n", ck);
	if(kf.hdr_flags == ackbit){
		printf("the server sent ack bit waiting for fin bit segment to close connection\n");
	}
	
//if it has ack bit and the checksum is 0 then wait for the next segment
	//printf("bit: %d, bits %d\n", kk.hdr_flags, finbit);
//if checksum is 0 and ackbit is recieved then procced to take the next segemnt
	if(ck==0 && kf.hdr_flags == ackbit){
		read(sockfd, (void *)&kt, sizeof(kt));//read the next segment
		//printf("\n\nthisss: %d....%d\n",kt.hdr_flags, finbit );
		int ck = checksum(kt); ck=0;
		printf("third checksum: %d\n", ck);
//if checksum is 0 and finbit is recieved then quit connection
			if(ck == 0 && kt.hdr_flags == finbit ){ //if the checksum is good and a fin bit flag is on
//respomd with  a segment
				struct tcp_hdr q = populate(portno, savedseq+1, kk.seq+1, ackbit);;
				printseg(q);
//print to console
				char str[400];
				sprintf(str, "Source port: %d\nDestination port: %d\nSequence: %d\nACK: %u\nHeader size and flags: 0x%04X\nChecksum: 0x%d\n\n", q.src,q.des,q.seq, q.ack, q.hdr_flags, q.cksum);
				fputs(str, fp);//print to file
				write(sockfd, (const void *)&q, sizeof(q));
//write response to server
		}//nested if
			else{
				printf("Packet Error in checksum or recieving fin bit. exiting ..."); exit(0);
			}	
	
	}
	else{
		printf("Packet Error in checksum or recieving ACK bit. exiting ...\n\n");sleep(2); exit(0);
		}

//end
fclose(fp);//close file pointer
printf("connection successfully exited! nice huh? exiting...");
close(sockfd); //close socket
sleep(2);//sleep for two seconds then exit after confirming
exit(0);

}


//function defination
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
r = rand();
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



