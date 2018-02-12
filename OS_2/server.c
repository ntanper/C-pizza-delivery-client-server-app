#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define SOCKET_NAME "socket.file" 
#define LISTEN_Q 10 //To megthos tis ouras anamonis gia sindeseis me ton server


/* Kathorizontai edw ws statheres oloi oi xronoi tou programmatos kathws kai ta plithi twn upallulwn (oi xronoi einai se sec)*/
#define tmargarita 0.01
#define tpepperoni 0.012
#define tspecial 0.015
#define tmakria 0.01
#define tkonta 0.005
#define tverylong 0.05
#define Nmaxpizza 3
#define Ndianomeis 10
#define Npsistes 10
#define Nmaxpizza 3
#define Nmaxorders 100 // O megistos arithmos paraggeliwn pou mporei na xeirizetai tautoxrona to sistima


/*Dhmiourgeitai enas pinakas megtehous Nmaxorders kai tupoy pthread_t. Etsi tha mporoume na xrisimopoihsoume tautoxrona tosa 
nhmata oses kai oi megistes paraggelies pou mporei na xeirizetai tautoxrona o server mas (dld Nmaxorders).*/
pthread_t threads[Nmaxorders];

int connect_sd, listen_sd; /*Oi socket descriptors stous opoious o server tha akouei ta aitimata sindesis (listen_sd) 
				   kai tha epitugxanei sindesi me ton kathe client (connect_sd) */

/*Orizoume ta mutex kai condition variables pou tha xrhsimopoihsoume.*/
pthread_mutex_t mut;
pthread_cond_t cond;

int Psistes,Dianomeis; //Metavlites pou dilonoun ton arithmo ton diathesimon psistwn kai dianomewn antistoixa
int blocked=0; //Metavliti pou xrisimopoieitai gia na dilwsei an kai posa nhamta exoun blockarei perimenontas psisti.
int blocked_del =0; //Metavliti pou xrisimopoieitai gia na dilwsei an kai posa nhamta exoun blockarei perimenontas dianomea.

/*H handle_pizza einai i sinartisi pou xeirizetai to psisimo mias pizzas. Gia na psisei mia pizza prepei na vrei enan diathesimo psisti. Gia
  logous sugxronismou ton nimatwn o kwdikas pou xeirizetai tin metavliti Psistes vrisketai se krisimi perioxi i opoia kleidwnetai kai 
  ksekleidwnetai me tin metavliti mutex (mut). Otan vrethei psistis kanoume sleep ton xrono pou xreiazetai gia na etoimastei i pizza. */
void *handle_pizza (void *p) {
	char pizza = (char)p;

	/*Kleidonoume to mutex wste na mpoume stin krisimi perioxi. Meionoume tous psistes kata 1 gia na dilosoume oti apasxoloume enan apo tous 
	  diathseimous psistes. An omws to Psistes einai <=0 shmainei oti den uparxei diathesimos psistis gia na paroume opote kai blockaroume me 
	  tin pthread_cond_wait i opoia kai ksekleidonei to mutex. Tha kseblokaroume otan kapoio allo nhma kalesei tin broadcast. Ayto tha to orisoume
	  na ginetai otan eleytherothei enas psistis apo ena nhma kai tautxorona gnorizoume oti uparxei blockarismeno nhma pou perimenei ton psisti 
	  ayton.*/	
	pthread_mutex_lock(&mut);
	while (Psistes<=0){
		blocked++;
		pthread_cond_wait(&cond,&mut);
		blocked--;
	}
	Psistes--;
	pthread_mutex_unlock(&mut);
	//sleep(5);
	/*Kanoume sleep gia xrono psisimatos analogo me ton tupo tis pizzas.*/
	if (pizza=='m')
		sleep(tmargarita);
	else if (pizza=='p')
		sleep(tpepperoni);
	else if (pizza=='s')
		sleep(tspecial);
	
	/*Twra tha ksanakleidosoume to mutex wste na auksisoume tin timi tis metavlitis Psistes mias kai den xreiazomaste pleon ton psisti. An
	  omws diapistosoume oti ,afou auksisame tin metavliti Psistes, i metavliti blocked exei tin timi true, kaloume tin pthread_cond_broadcast gia na
	  kseblockaroume to nima pou exei blockarei (broadcast anti gia signal otan uparxoun panw apo ena blockarismena nhmata).*/ 
	pthread_mutex_lock(&mut);
	Psistes++;
	if (blocked==1) 
		pthread_cond_signal(&cond);
	else if (blocked>1)
		pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mut);
	pthread_exit(NULL);
}

/*H sinartisi handle_client pou kaolouthei einai i sinartisi pou kaleitai otan dhmiourgeitai ena neo thread apo thn main. Skopos tis einai
  na lavei tin paraggelia apo ton client kai na tin ektelesei. Afou lavei tin paraggelia tou pelati tha dhmiourgisei me tin seira tis nea 
  nhmata (ena gia kathe pizza tis paraggelias) ta opoia tha analavoun na psisoun tin kathe pizza. Afou psithoun oles oi pitses, i handle_client
  analamvanei tin dianomi tis paraggelias ston pelati.*/
void *handle_client(void *id) {
	char buffer[50];
	char dist[4];
	char pizzas[Nmaxpizza];
	pthread_t threads2[Nmaxpizza];
	pthread_attr_t attr;
	int j=0;	
	bzero(buffer,50);
	read(connect_sd,buffer,sizeof(buffer));
	sscanf(buffer, "%s %s", &dist,&pizzas);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);	
	/*Afou exoume lavei tin paraggelia, dhmiourgoume ena nhma gia na xeiristei tin kathe pizza ksexorista. Sti sinexeia perimenoume
	  auta ta nhmata na teleiosoun kalontas tin join wste na proxorisoume stin dianomi tis paraggelias.*/
	  	
	for (j=0;j<strlen(pizzas);j++) {
		pthread_create(&threads2[j],&attr,handle_pizza,(void*)pizzas[j]);
	}
	for (j=0;j<strlen(pizzas);j++) 
		pthread_join(threads2[j],NULL);
	
	/*Kleidonoume to mutex wste na mpoume stin krisimi perioxi. Meionoume tous dianomeis kata 1 gia na dilosoume oti apasxoloume enan apo tous 
	  diathseimous dianomeis. An omws to Dianimeis einai <=0 shmainei oti den uparxei diathesimos dianomeas gia na paroume opote kai blockaroume me 
	  tin pthread_cond_wait i opoia kai ksekleidonei to mutex. Tha kseblokaroume otan kapoio allo nhma kalesei tin broadcast. Ayto tha to orisoume
	  na ginetai otan eleytherothei enas dianomeas apo ena nhma kai tautxorona gnorizoume oti uparxei blockarismeno nhma pou perimenei ton 
	  dianomea ayton.*/	
	pthread_mutex_lock(&mut);	
	while (Dianomeis<=0){
		blocked_del++;
		pthread_cond_wait(&cond,&mut);
		blocked_del--;
	}
	Dianomeis--;
	pthread_mutex_unlock(&mut);
	
	/*Kanoume sleep gia xrono dianmois analogo me tin apostasi.*/
	if (dist=="near")
		sleep(tkonta);
	else if (dist=="far")
		sleep(tmakria);

	/*Twra tha ksanakleidosoume to mutex wste na auksisoume tin timi tis metavlitis Dianomeis mias kai den xreiazomaste pleon ton dianomea. An
	  omws diapistosoume oti ,afou auksisame tin metavliti Dianomeis, i metavliti blocked_del exei tin timi true, kaloume tin pthread_cond_broadcast 
	  gia na kseblockaroume to nima pou exei blockarei (broadcast anti gia signal otan uparxoun panw apo ena blockarismena nhmata).*/
	pthread_mutex_lock(&mut);
	Dianomeis++;
	if (blocked_del==1) 
		pthread_cond_signal(&cond);
	else if (blocked_del>1)
		pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mut);
	printf("I am thread No %d and I delivered order: %s\n",*my_id,pizzas);
	/*Stelnetai telos minima oti i paragglia exei stalei epityxws.*/
	write(connect_sd,"Your order has been delivered\n",30);	
	close(connect_sd);
	pthread_exit(NULL);
}

int main () {
		
	struct sockaddr_un server_addr, client_addr; //Structs pou periexoun plirofories sxetika me tis dieuthinseis tou server kai tou client antistoixa
	socklen_t client_len; //To megethos tis dieuthinsis tou client
	
	
	unlink (SOCKET_NAME);
	
	
	/*Arxikopoioume ta antikeimena mutex kai condition*/
	pthread_mutex_init(&mut,NULL);
	pthread_cond_init(&cond,NULL);

 
	/*Arxika o arithmos twn diathesimwn psistwn kai dianomewn ginetai isos me ton sunoliko arithmo twn psistwn kai dianomewn antistoixa */
	Psistes=Npsistes;
	Dianomeis=Ndianomeis;	
	
	/*Dhmiourgia to listen_sd: AF_UNIX gia epikoinvnia metaksi diergasiwn stin idia mixani
				   SOCK_STREAM gia roi dedomenvn xwris oria megethous  
	Tautoxrona ginetai elegxos gia to an i dhmiourgia htan epityxhs. Ektypvnetai sfalma an htan anepityxhs kai o server termatizei*/
	if ((listen_sd = socket(AF_UNIX, SOCK_STREAM, 0))<0) {
		printf("Failed to create socket\n");
		exit(1);
	}
	
	/* Arxika tha katharisoume (metin bzero) tin struct server_addr 
	kai sti sinexeia tha dothoun se autin oi katallhles times tis deiuthinsis tou server (oikegeneia AF_UNIX kai onoma to prokathorismeno)*/
	bzero(&server_addr, sizeof( server_addr ));	
	server_addr.sun_family = AF_UNIX;
	strcpy( server_addr.sun_path, SOCKET_NAME );
	
	/* Twra tha ginei bind o socket descriptor listen_sd kai tha oristei ws to socket sto opoio o server tha "akouei" aithmata sindesis 
	Tautoxrona tha ginei elegxos gia to an egine swsta i ektelesi tis bind. Diaforetika tha emfanistei minima sfalmatos. */
	if (( bind( listen_sd, ( struct sockaddr* )&server_addr, sizeof( server_addr)))<0) {
		printf("Failed to bind socket\n");
		exit(1);
	}
	
	/* Sti sinexeia tha kalesoume to system call listen oste na dilosoume oti o server "akouei" aitimata me megisti oura anamonis LISTEN_Q 
	Se periptosi sfalamtos ginontai ta gnwsta */
	if ((listen( listen_sd, LISTEN_Q))<0) {
		printf("Listen system call failed\n");
		exit(1);
	}
		

	/* Mpainoume twra se ena aenao loop. Etsi eksasfalizoume oti o server tha trexei sinexeia sto background xwris na termatizei.
	Ston kwdika pou uparxei mesa se auto to loop tha ginontai oi katallhles diergasies wste na:
	-Epitygxanetai sindesi me enan client.
	-Dhmiourgeitai ena neo thread pou tha xeirizetai ton client
	-Epistrefei o server se katastasi anamonis perimenontas ton epomeno client. */	
	for (;;) {
		char buffer[50];				
		char pizzas[Nmaxpizza],dist[4];		
		client_len=sizeof(client_addr);		
		int found;
		
		/* Me tin accept epitugxanetai sindesi metaksi server kai client. Otan o server sinantisei tin accept perimenei ekei.
		H roi tou programmatos sinexizetai mono otan klithei i connect apo ton client */ 		
		connect_sd = accept (listen_sd,( struct sockaddr * ) &client_addr, &client_len);  	
		

		 
		/* Afou exei epiteyxthei sindesi, kaleitai pthread_create wste na dhmiourgithei neo nhma to opoio tha xeirsitei ton client.*/
			
		int rc;		
		printf("I=%d\n",i);		
		rc = pthread_create(&threads[i],NULL,handle_client,(void *)&i);
		if (rc) {
			printf("ERROR; return code from pthread_create is %d\n",rc);
			exit(-1);
		}
		i++;
		
	}
}






	
		





