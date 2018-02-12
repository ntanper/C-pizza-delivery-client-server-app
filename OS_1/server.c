#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SOCKET_NAME "socket.file" 
#define LISTEN_Q 10 //To megthos tis ouras anamonis gia sindeseis me ton server
//#define SHM_SIZE 1024 //To megethos tis koinis mnimis
#define SHM_KEY "my_key" //To kleidi tis koinis mas mnimis
#define SEM_NAME "my_semaphore"

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

int main () {
	int connect_sd, listen_sd; /*Oi socket descriptors stous opoious o server tha akouei ta aitimata sindesis (listen_sd) 
				   kai tha epitugxanei sindesi me ton kathe client (connect_sd) */
	pid_t child_pid; // Ta process ids pou epistrefontai apo tin fork	
	struct sockaddr_un server_addr, client_addr; //Structs pou periexoun plirofories sxetika me tis dieuthinseis tou server kai tou client antistoixa
	socklen_t client_len; //To megethos tis dieuthinsis tou client
	int shm_id; //To id tis koinis mnimis
	char *data; //Deiktis sta stoixeia tis  koinis mnimis
	int P[Npsistes],D[Ndianomeis];//Pinakes pou tha dilwnoun tin katastasi twn upallilwn
	int i;

	unlink (SOCKET_NAME);
	
	/*Orizetai i domi tupou order i opoia tha xrisimopoiethei gia tin apothikeusi twn paraggeliwn. Periexei to string distance sto opoio 
	  tha dilwnetai an o pelatis einai konta i makria, ena string pizza_list pou tha periexei tin lista me tis pizzes tis paragglias
	  (1 xaraktiras gia kathe pizza) kai to int coca pou einai 1 an stin paraggelia dothei dwro coca-cola kai 0 diaforetika. */
	typedef struct {
		char distance[5];
		char pizza_list[Nmaxpizza];
		int coca;	
	} order;

	order my_orders[Nmaxorders]; //Pinakas kathe stoixeio tou opoiou tha einai mia paraggelia
	order *p,*top; //deiktes se domi tipou order pou tha xrisimopioithoun gia prospelasi twn dedomenwn
 
	/*Kathe stoixeio twn pinakwn P kai D tha exei tin timo 0 h 1. 0 shmainei oti o antistoixos upallilos einai diathesimos kai 
	  1 oti einai apasxolimenos. Arxika oles oi times einai 0 (oloi diathesimoi) */		
	for (i=0;i<Npsistes;i++)
		P[i]=0;	
	for (i=0;i<Ndianomeis;i++) 
		D[i]=0;
	
	
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
	
	
	int shm_size;
	shm_size=Npsistes+Ndianomeis+sizeof(my_orders);
	printf("SHM_SIZE:%d\n",shm_size);
	/* Tha dhmiourgisoume twra tin koini mnimi kalontas tin shmget. Tautoxrona ginetai kai elegxos sfalmatos
	   To shm_flag pedio einai 0600 | IPC_CREAT kathos eimaste stin diergasia pou 			
	   dhmiourgei tin kuria mnimi (To 0600 dilonei tin adeia pou tha exei i diergasia	
   	   stin koini mnimi)*/
	if ((shm_id=shmget(SHM_KEY,shm_size,0600 | IPC_CREAT))<0) { 
		printf("shm_id= %d\n", shm_id);		
		printf("Could not create shared memory\n");
		exit(1);
	}
	
	/* Twra tha proskolisoume tin diergasia mas stin koini mnimi me tin shmat. H shmat epistrefei deikti stin arxi tis koinis mnimis.*/
	if ((data=shmat(shm_id,NULL,0))==(char *)-1) {
		printf("Could not attach to shared memory\n");
		exit(1);
	}	

	/*O deiktis se struct ,p , arxika tha deixnei meta tis theseis twn psistwn kai twn dianomewn stin koini mnimi
	  enw o top mia thesi meta.*/
	p=data+Npsistes+Ndianomeis;
	top=p+1;

	/* Tha apothikeusoume twra tous duo pinakes pou kratane tin katastasi tvn upallilwn (P kai D) stin koini mnimi,wste na mporoun na
	tous xeiristoun oles oi diergasies pou meirazontai tin mnimi */
	for (i=0;i<Npsistes;i++)//Kathe timi tou pinaka P mpainei se diadoxikes theseis koinis mnimis stis opoies deixnei to data+i		
		*(data+i)=P[i];
	for (i=0;i<Ndianomeis;i++) //Kathe timi tou pinaka D mpainei se diadoxikes theseis pou ksekinoun meta tis theseis pou antisotoixoun ston pinaka P
		*(data+Npsistes+i)=D[i];

		
	/* Mpainoume twra se ena aenao loop. Etsi eksasfalizoume oti o server tha trexei sinexeia sto background xwris na termatizei.
	Ston kwdika pou uparxei mesa se auto to loop tha ginontai oi katallhles diergasies wste na:
	-Epitygxanetai sindesi me enan client.
	-Dhmiourgeitai mia diergasia-paidi pou tha xeirizetai ton client
	-Epistrefei o server se katastasi anamonis perimenontas ton epomeno client. */	
	
	for (;;) {
		char buffer[50];				
		char pizzas[50],dist[4];		
		client_len=sizeof(client_addr);		
		sem_t *my_sem;
		
		/* Me tin accept epitugxanetai sindesi metaksi server kai client. Otan o server sinantisei tin accept perimenei ekei.
		H roi tou programmatos sinexizetai mono otan klithei i connect apo ton client */ 		
		connect_sd = accept (listen_sd,( struct sockaddr * ) &client_addr, &client_len);  	
		
		/*Dhmiourgoume ena shmaforo pou tha xrisimopoiithei apo tis diergasies paidia gia prosvasi stin koini mnimi. */
		my_sem = sem_open(SEM_NAME, O_CREAT | O_RDWR,S_IRUSR | S_IWUSR, 1);
		if (my_sem == SEM_FAILED) {
			printf("Could not open semaphore!\n");
			exit(1);
		}

		
		
		/*O deiktis p arxika deixnei sto data diladi stin arxi tis koinis mnimis. Me tin do-while pou akolouthei tha 
		  diapistothei poia thesi mnimis tha xrisimopoithei gia tin apothikeusi tis paraggelias. An to stoixeio distance 
	          tis thesis autis einai far i near tote einai katillimeni apo alli paraggelia. An oxi kanoume break kai 
		  kratame auto to p(pou deixnei se keni thesi mnimis) oste na to xrisimopoiisoume stin diergasia paidi pou tha 
	   	  xeiristei tin paraggelia. An einai katillimeni ayksanoume to p kata 1 oste na deiksei stin epomeni thesi mnimis
	 	  kai elegxoume authn. Ayto ginetai mexri na ftasoume stin thesi top pou dilonei tin thesi mnimis stin opoia exei 
		  apothikeutei i teleutaia paraggelia. An ftasoume se auti tin thesi auxanoume to top kata 1. */
		p=data+Npsistes+Ndianomeis;
		do { 
		/*Kanoume ena sem_wait kai duo sem_post(ena mesa sthn if kai ena ekso apo autin). 
               	  Eite mpoume stin if eite oxi tha klithei ena sem_post.*/
			sem_wait(my_sem);			
			if ((strcmp(p->distance,"near")!=0)&&(strcmp(p->distance,"far")!=0)) {
				sem_post(my_sem);				
				break;
			}
			sem_post(my_sem);
			p++;
		}while (p!=top);
		if (p==top) 
			top++;
					
		/* Afou exei epiteyxthei sindesi, kaleitai i fork wste na dhmiourgithei diergasia paidi i opoia tha xeirsitei ton client.
		An to pid exei tin timi 0 tote eimaste stin diergasia paidi opote kane tis aparaithtes energeies.
		An to pid exei timi != 0 tote eimaste stin diergasia patera opote min kaneis tipota kai epestrepse mesw tou loop stin accept */
		if ((child_pid = fork())==0) {
			int shm_id;	
			char *data;
			pid_t childs;
			int l=0;
			/*Kathe diergasia paidi tha kanei shmget kai shmat wste na parei tin koini mnimi pou dhmiourghse 
			  i diergasia pateras kai na proskoleithei se autin*/			
			shm_id = shmget(SHM_KEY,shm_size,0600);
			data = shmat(shm_id,NULL,0);		

			/*Tha kanoume twra mia read gia na diavasoume kai na apothikeusoume ston buffer tin parragelia (order)*/
			bzero(buffer,50);			
			read(connect_sd,buffer,sizeof(buffer));				
			sscanf(buffer, "%s %s", &dist,&pizzas);

			/*Afou exoume lavei tin paraggelia tha tin apothikeusoume stin koini mnimi stin katallili thesi diladi stin
	 		  thesi pou deixnei o deiktis p. */
			sem_wait(my_sem);			
			strcpy(p->distance,dist);
			strcpy(p->pizza_list,pizzas);
			p->coca=0;
			sem_post(my_sem);

			/*Gia kathe pizza pou vrisketai stin paraggelia mas tha dimiourgithei mia diergasia paidi gia na tin xeiristei.
			  Xrisimopoiontaws ton metriti tis for, i, kathe paidi tha analavei tin i-osti pizza tis paraggelias kai etsi
			  apofeygoume to na douleyoun 2 diergasies panw stin idia pizza. */
			for (l=0;l<strlen(pizzas);l++) { 				
				if ((childs=fork())==0) {

					/*Kathe diergasia paidi tha kanei shmget kai shmat wste na parei tin koini mnimi pou dhmiourghse 
			 		  i diergasia pateras kai na proskoleithei se autin*/			
					shm_id = shmget(SHM_KEY,shm_size,0600);
					data = shmat(shm_id,NULL,0);		
					int j=0;
										
					/*Tha mpoume twra se mia epanalipsi i opoia tha psaxnei olous tous psistes mesw
					  tou pinaka P mexri na vrei enan diathesimo (*(data+j)==0).
					  Molis ton vrei, anoigei shmaforo gia na kanei tin timi tou 1 (*(data+j)=1).
	                 		  An den vrei diathesimo psisti ksanapsaxnei apo tin arxi mexri na vrei 
					  (mexri kapoia alli diergasia na eleutehrosei enan. */					
					while(1) {
						sem_wait(my_sem);
						if (*(data+j)==0) {
 							*(data+j)=1;
							sem_post(my_sem);
							break;
						}
					sem_post(my_sem);					
					j++;
					if (j==Npsistes)
						j=0;						
					};					
					
					/*Afou exoume vrei poios psistis tha psisei tin sigkekrimeni pizza , elegxoume to eidos tis 
				 	  pitsas kai kanoume sleep gia antistoixo xrono wste na psithei i pizza. */
					if (pizzas[l]=='m') 
						 sleep(tmargarita);
					else if (pizzas[l]=='p')
 						sleep(tpepperoni);
					else if (pizzas[l]=='s')
						sleep(tspecial);	
					
					/*Afou psisame tin pizza tha anoiksoume shmaforo gia na allaksoume ta dedomena tis koinis mnimis.
					  Epanaferoume ton psisit ws diathesimo (*(data+j)=0;) kai thetoume tin pizza ws etoimi
					  (p->pizza_list[l]='r'). */
					sem_wait(my_sem);
					*(data+j)=0;
					p->pizza_list[l]='r';						
					sem_post(my_sem);
					
					/*H diergasia pou anelave to psisimo tis mias pizzas teleiose. Kleinoume ton shmaforo,
				 	  apokoloumaste apo thn koini mnimi kai termatizoume. */
					sem_close(my_sem);					
					shmdt(data);					
					exit(1);
				}		
			}
			
			/*I diergasia mas exei pleon dhmiourgisei ena paidi gia kathe pizza tis paraggelias. Twra perimenei na teleiosoun
			  wste na steilei tin paraggelia me kapoion dianomea. Tha kserei oti exoun psithei oles oi pitses otan oi times 
			  tou pizzas[i] einai oles 'r'. Parakatw elegxontai oles oi times tou pinaka pizzas kai an estw kai mia den exei
			  tin timi 'r' tote i paraggelia den einai etoimi (ready==0). Ksanaelegxei tis times mexris otou telika na einai 
			  oles etoimes diladi mexri ready==1. */

			int k=0,ready=0;
						
			do {
				ready=1;
				for (k=0;k<strlen(pizzas);k++) {
					sem_wait(my_sem);					
					if (p->pizza_list[k]!='r') 
						ready=0;
					sem_post(my_sem);
				}
			}while (ready==0);
			
			/*Twra exoume etoimasei olokliri tin paraggelia kai tha psaksoume na vroume enan diathesimo dianomea gia na tin 
			  metaferei. I diadikasia psaximatos dianomea einai idia me to psaksimo gia psisti pou egine proigoumenos.
			  Molis vrethei diathesimos dianomeas kanoume sleep gia oso xreiazetai i dianomi (tmakria i tkonta). */
			i=0;		
			while (1) {
				sem_wait(my_sem);
				if ((*(data+Npsistes+i))==0) {					
					*(data+Npsistes+i)=1;
					sem_post(my_sem);
					break;
				}
				sem_post(my_sem);
				i++;
				if (i==Ndianomeis)
					i=0;
			};
			if (dist=="far") 
				sleep(tmakria);
			else if (dist=="near")
				sleep(tkonta);				
			sem_wait(my_sem);
			*(data+Npsistes+i)=0;
			sem_post(my_sem);
			
			/*Stelnetai telos minima oti i paragglia exei stalei epityxws.*/
			write(connect_sd,"Your order has been delivered\n",30);
			
			/*Telos prin i diadikasia termatisei svinei apo tin koini mnimi tin paraggelia pou esteile.*/
			sem_wait(my_sem);
			bzero(p->distance,sizeof(p->distance));	
			bzero(p->pizza_list,sizeof(p->pizza_list));
			p->coca=0;
			sem_post(my_sem);	
			
			sem_close(my_sem);			
			shmdt(data);			
			close(connect_sd);		
			exit(1);	
		}
	
	}
	sem_unlink(SEM_NAME);
	

	/* Apokollisi tou server apo tin koini mnimi me th shmdt. */
	if ((shmdt(data)) == -1) {
		printf("Could not detach from shared memory\n");	
		exit(1);
	}
	
	/*Diagrafi tis koinis mnimis.*/
	shmctl(shm_id,IPC_RMID,NULL);
}
