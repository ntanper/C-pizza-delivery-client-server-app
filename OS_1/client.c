#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/un.h>

#define SOCKET_NAME "socket.file"
#define Nmaxpizza 3 //O megistos arithmos pizzas pou mporei kaneis na parageilei

int main(int argc, char *argv[]) { /*Dinoume tin dinatotita ston pelati na dwsei tin paraggelia kateutheian apo tin grammi entolwn.
				     Tha prepei na dwsei 4 orismata peran tou onomatos tou programmatos (argc=5).
				     To prwto orisma (argv[1]) tha exei tin timi FAR h tin timi NEAR kai dilwnei to an o pelatis einai 
				     makrinos i kontinos.
				     To 2o orisma einai enas arithmos kai dilwnei ton arithmo twn pizza tupou margarita, to 3o ton arithmo twn pepperoni
				     kai to 4o ton arithmo twn special */
	
	char cl_distance[4]; //Apothikeuei to an o pelatis einai makrinos i kontinos
	int num_mar=0, num_pep=0,num_spe=0; //O arithmos ton pizzas tupou margarita, pepperoni kai special antistoixa
	int choice;
	char order[50]; //Edw tha apothikeutei i paraggelia (olokliri se ena string) gia na stakei ston server	
	bzero(cl_distance,4);

	/*Tha ginei elegxos gia to an dothikan orismata. An nai tote oi metavlites mas tha paroun times apo ta orismata auta. 
	  An oxi tha paroun times apo ena menu erotisewn pou tha akolouthisei.*/
	if (argc==5) {
		strcpy(cl_distance,argv[1]);
		sscanf(argv[2],"%d",&num_mar);
		sscanf(argv[3],"%d",&num_pep);
		sscanf(argv[4],"%d",&num_spe);
	}
	else if (argc==1) { //Dothike os orisma mono to onoma tou programmatos kai ara i paraggelia tha ginei meso enos menou erwthsewn/apanthsewn.
		printf("Hello and welcome to our pizza delivery system\n");
		printf("First, are you located near or far from our store?(Type 1 or 2)\n1.Near\n2.Far\n");
		scanf("%d",&choice);		
		if (choice==1) 
			strcpy(cl_distance,"near");
		else 
			strcpy(cl_distance,"far");
		printf("You can order a maximum of %d pizzas\n",Nmaxpizza);
		printf("How many margaritas would you like?\n");
		scanf("%d",&num_mar);
		printf("How many pepperoni would you like?\n");
		scanf("%d",&num_pep);
		printf("How many special would you like?\n");
		scanf("%d",&num_spe);
		printf("Thank you! We will now prepare your order\n");
	}	
	
	/*Twra exoume sileksei (mesw grammis entolwn h mesw tou menu) tis aparaithtes plirofories tis paraggelias.
	  Tha ftiaxoume ena string pou na periexei oles autes tis plirofories wste na to steiloume mesw tou socket ston server.
	  To string auto (order) tha exei tin morfi order = "far mmps". Diladi far(h near) gia tin apostasi kai mmps dilwnei: 
	  Margarita Margarita Pepperoni Special dhladh ti eidous pitses thelei o pelatis. I kodikopoiisi auti ginetai 
	  gia na stalei pio eukola i paraggelia ston server. */
	
	int i=0;
	bzero(order,50);
	strcpy(order,cl_distance); //Arxika to order periexei to far i to near
	strcat(order, " ");//Prosthetoume ena keno
	for (i=0;i<num_mar;i++) //Prosthetoume ena m gia kathe margarita, ena p gia kathe pepperoni kai ena s gia kathe special
		strcat(order,"m");
	for (i=0;i<num_pep;i++) 
		strcat(order,"p");
	for (i=0;i<num_spe;i++) 
		strcat(order,"s");
	
	/*To stirng order exei pleon tin katallili morfi kai einai etoimo na stalei. Dhmiourgoume to socket, dinoume tis katalliles times stin 
	  struct me tin diuthinsi tou server kai kanoume connect. */

	int client_sd;
	client_sd = socket(AF_UNIX,SOCK_STREAM,0);
	struct sockaddr_un server_addr;
	bzero(&server_addr, sizeof( server_addr ));	
	server_addr.sun_family = AF_UNIX;
	strcpy( server_addr.sun_path, SOCKET_NAME );
	connect(client_sd,(struct sockaddr *) &server_addr, sizeof(server_addr));
		
	write(client_sd,order,sizeof(order));
	bzero(order,50);
	read(client_sd,order,sizeof(order));
	printf("%s\n",order);
	close(client_sd);
}
