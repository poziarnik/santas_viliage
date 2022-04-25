#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h> //flagy
#include <stdbool.h>

void createCreatures(int* mainid,int* ID, char* whatAreU,int NumberOf, char* name);
void unlinkSemaphores();
bool isNumber(char* string);
bool Test(int argc, char* argv[]);

// Moj kod je prudko nedorobeny a trochu sa hanbim odovzdat 250 riadkovy main :(
// Snazil som sa ho prepisat ale nedarilo sa mi moc a uz mi neostal cas... 
// Dakujem za opravu. Pekny den. 

int main(int argc, char* argv[]) 
{
    if(!Test(argc, argv)){
        return 1;
    }
    unlinkSemaphores();
    FILE *out = fopen("proj2.out", "w");

    setbuf(out, NULL);

    pid_t id = 1;                   //inicializovanie id na aby sa spustil prvy fork
    
    int ID = 0;                     //premenne pre child procesy
    char whatAreU [10];

    sem_t *sem1 = sem_open("sem1", O_CREAT, 0644, 1);          //tryout
    sem_t *semDoor = sem_open("semDoor", O_CREAT, 0644, 0);
    sem_t *semSanta = sem_open("semSanta", O_CREAT, 0644, 0);
    sem_t *mutex = sem_open("mutex", O_CREAT, 0644, 1);
    sem_t *semElfsDone = sem_open("semElfsDone", O_CREAT, 0644, 0);
    sem_t *semSobCount = sem_open("semSobCount", O_CREAT, 0644, 1);
    sem_t *semSobWaitforSanta = sem_open("semSobWaitforSanta", O_CREAT, 0644, 0);
    sem_t *semSobOrElf = sem_open("semSobOrElf", O_CREAT, 0644, 1);
    sem_t *semPrint= sem_open("semPrint", O_CREAT, 0644, 1);
    sem_t *semHitch= sem_open("semHitch", O_CREAT, 0644, 0);
    
      

    shm_unlink("countObj");
    int fd;
    fd = shm_open("countObj", O_CREAT | O_RDWR,0600);       //premenna ktora pocita pocet elfou v dielni
    ftruncate(fd, sizeof(int));
    int * count = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);           //shared mem
    *count = 0;

    shm_unlink("countSob");
    int fd2;
    fd2 = shm_open("countSob", O_CREAT | O_RDWR,0600);       //premenna ktora pocita pocet sobov na dovolenke
    ftruncate(fd2, sizeof(int));
    int * countSob = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd2,0);           //shared mem
    *countSob = atoi(argv[2]);

    shm_unlink("workshopClosed");
    int fd3;
    fd3 = shm_open("workshopClosed", O_CREAT | O_RDWR,0600);       //premenna ktora pocita pocet sobov na dovolenke
    ftruncate(fd3, sizeof(int));
    int * workshopClosed = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd3,0);           //shared mem
    *workshopClosed = 0;

    shm_unlink("countLines");
    int fd4;
    fd4 = shm_open("countLines", O_CREAT | O_RDWR,0600);       //premenna ktora pocita pocet sobov na dovolenke
    ftruncate(fd4, sizeof(int));
    int * countLines = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd4,0);           //shared mem
    *countLines = 0;
    
    
    createCreatures(&id, &ID, whatAreU, 1, "Santa");
    if (id!=0)
    {
        createCreatures(&id, &ID, whatAreU, atoi(argv[1]), "Elf");
    }
    if (id!=0)
    {
        createCreatures(&id, &ID, whatAreU, atoi(argv[2]), "RD");
    }
    
    /*if(id==0){                                  //musel som to vytiahnut z createCreatures funkcie koli unused variable :(
        sem_t *sem1 = sem_open("sem1",0);       
        sem_t *mutex = sem_open("mutex",0);  
        sem_t *semSanta = sem_open("semSanta",0);  
        sem_t *semDoor = sem_open("semDoor",0);   
        sem_t *semElfsDone = sem_open("semElfsDone",0);   
        sem_t *semSobCount = sem_open("semSobCount",0);
        sem_t *semSobWaitforSanta = sem_open("semSobWaitforSanta", 0);
        sem_t *semSobOrElf = sem_open("semSobOrElf",0);
        sem_t *semPrint = sem_open("semPrint",0);
        sem_t *semHitch = sem_open("semHitch",0);
    }*/

    if(strcmp(whatAreU,"Elf") == 0){
    sem_wait(semPrint);
    fprintf(out,"%d: %s %d: started\n",*countLines=*countLines+1,whatAreU, ID);
    sem_post(semPrint);
    while(1){
            //printf("im here %s %d\n",whatAreU, ID);
            usleep(atoi(argv[3]));
            sem_wait(semPrint);                 //vzdy ked zapisujem musim zaistit aby naraz nazapisovalo viacej procesov
            fprintf(out,"%d: %s %d: need help\n",*countLines=*countLines+1,whatAreU, ID);
            sem_post(semPrint);
            sem_wait(mutex);
            sem_wait(semSobOrElf);              //ak posledny sob zobudza santu tak sa ani nezaraduj zatial

            if (*workshopClosed==1){            //ak je zatvorena dielna psuti dalsieho aby sa pozrel a zrus sa
                sem_post(mutex);                //pusti dalsieho
                sem_post(semSobOrElf);
                sem_wait(semPrint);
                fprintf(out,"%d: %s %d: taking holidays\n",*countLines=*countLines+1, whatAreU, ID);
                sem_post(semPrint);
                break;
            }
            else{

                *count = *count + 1;
                if (*count==3){
                    sem_post(semSanta);             //ak traja cakate pred dielnou tak zaobud santu aby vam pomohol
                }
                else{
                    sem_post(semSobOrElf);          //ak posleny sob caka aby zobudil santu tak moze ist na to  
                    sem_post(mutex);                //ak nie ste traja pusti dalsieho
                }
                sem_wait(semDoor);                  //cakaj kym santa nepomoze
                if (*workshopClosed==1){            //ak je zatvorena dielna psuti dalsieho aby sa pozrel a zrus sa
                    sem_post(mutex);                //pusti dalsieho
                    sem_post(semSobOrElf);
                    sem_wait(semPrint);
                    fprintf(out,"%d: %s %d: taking holidays\n",*countLines=*countLines+1, whatAreU, ID);
                    sem_post(semPrint);
                    break;
                }
                sem_wait(sem1);                     //po jednom vychadzate z dielne
                *count = *count - 1;
                sem_wait(semPrint);
                fprintf(out,"%d: %s %d: get help\n",*countLines=*countLines+1, whatAreU, ID);
                sem_post(semPrint);
                if (*count==0){                     //ak si posledny
                    sem_post(semElfsDone);          //uvoli cakajuceho santu ktory chce spat
                }
                else{
                    sem_post(sem1);                 //ak nie si posledny uvolni semafor pre dalsieho
                }
            }
        }

        unlinkSemaphores();
        exit(0);
    }
    
    if(strcmp(whatAreU,"Santa") == 0){
        sem_wait(semPrint);
        fprintf(out,"%d: %s: going to sleep\n",*countLines=*countLines+1,whatAreU);
        sem_post(semPrint);
        while(1){
            sem_wait(semSanta);                       //cakaj kym budu traja pred pripraveny alebo kym sa vsetky soby nevratia
            if(*countSob==-1){                        //ak sa vratia vsetky soby
                sem_wait(semPrint);
                fprintf(out,"%d: %s: closing workshop \n",*countLines=*countLines+1, whatAreU);
                sem_post(semPrint);
                *workshopClosed=1;
                
                sem_post(mutex);                     //uvolni dalsich elfov
                sem_post(semSobOrElf);
                
                sem_post(semDoor);                   //elfovia ktory su zaradeny pred branou idu na praydniny
                sem_post(semDoor);
                for (int i = 0; i < atoi(argv[2]); i++)        //zapriahni vsetky sobi
                {
                    sem_post(semSobWaitforSanta);
                    sem_wait(semHitch);                         //vzdy cakaj kym sa zapriahne
                }
                
                sem_wait(semPrint);
                fprintf(out,"%d: %s: Christmas started\n",*countLines=*countLines+1, whatAreU);
                sem_post(semPrint);
                
                
                break;
            }
            else{                                     //ak sa elfovia pred branov zhromazdia 3ja elfovia
            sem_wait(semPrint);
            fprintf(out,"%d: %s: helping elves\n",*countLines=*countLines+1, whatAreU);
            sem_post(semPrint);
            sem_post(semDoor);                        //otvor trom branu
            sem_post(semDoor);
            sem_post(semDoor);
            sem_wait(semElfsDone);                    //cakaj kym vsetci traja prejdu
            sem_wait(semPrint);
            fprintf(out,"%d: %s: going to sleep\n",*countLines=*countLines+1,whatAreU);
            sem_post(semPrint);
            sem_post(sem1);                           //uvolni po tretom semafor
            sem_post(semSobOrElf);                    //ak posleny sob caka aby zobudil santu tak moze ist na to  
            sem_post(mutex);                          //uvolni mutex pre dalsieho co caka pred branou
            }
        }


        unlinkSemaphores(); 
        exit(0);
    }
    if(strcmp(whatAreU,"RD") == 0){
        sem_wait(semPrint);
        fprintf(out,"%d: %s %d: rstarted\n",*countLines=*countLines+1,whatAreU, ID);
        sem_post(semPrint);
        usleep(atoi(argv[4]));
        //printf("som tu %s %d\n\n",whatAreU, ID);
        sem_wait(semSobCount);
        sem_wait(semPrint);
        fprintf(out,"%d: %s %d: return home\n",*countLines=*countLines+1, whatAreU, ID);
        sem_post(semPrint);
        *countSob=*countSob - 1;
        if (*countSob==0){
            sem_wait(semSobOrElf);                  //ak sa prave zaraduje elf alebo santa pomaha elfom tak este chvilu pockaj
            *countSob=*countSob - 1;                //spusti podmienku -1
            sem_post(semSobCount);
            sem_post(semSanta);
        }
        else{
            sem_post(semSobCount);
        }
            
        sem_wait(semSobWaitforSanta);               //santa po jedno zaprahufa soby
        sem_wait(semPrint);
        fprintf(out,"%d: %s %d: get hitched\n",*countLines=*countLines+1, whatAreU, ID);
        *countSob=*countSob + 1;
        sem_post(semPrint);
        sem_post(semHitch);
        
        

        unlinkSemaphores(); 
        exit(0);
    }


    if(id != 0){
        while(wait(NULL)>0);
        unlinkSemaphores(); 
        //koniec hlavneho procesu
    }

    munmap(count, sizeof(int));                                             //zrusenie vsetkeho
    shm_unlink("countObj");
    shm_unlink("countSob");
    shm_unlink("workshopClosed");
    shm_unlink("countLines");
    
    unlinkSemaphores();                                                        //tiez ich asi staci unlinknut raz...

    
    return 0;
}
//prvy parameter je id procesu ktory spusta funkciu, 2. a 3. su znaky specificke pre kazdy proces ktory funkciu opusti,4. je pocet vytvorenych procesov, 5. je meno vytvorenych vianocnych stvoreni
void createCreatures(int* pid,int* ID, char* whatAreU,int NumberOf, char* name){         
    for(int i = 1;i <= NumberOf;i++){       //vytvorim pozadovany pocet procesov
        *pid=fork();                    //vytvori sa kopia procesu ktory funkciu spustil
        if (*pid == 0){                 //procesi vytvaram len z procesu ktory funkciu spustil, kazdy vytvoreny proces child bude mat mainid==0 takze uz v ziadno dalsiom chode cyklu podmienku nespusti
            strcpy(whatAreU, name);         //proces sa pomenuje
            *ID=i;                          //procesu sa priradi cislo(to bude pre kazdy proces specificke,lebo vytvoreny proces sa do if znovu nedostane a opusti funkciu s nastavenou ID)
            break;                          //child proces vyskoci z cyklu, parent proces ostava v cykle
        }
    }
}
void unlinkSemaphores(){
    
    sem_unlink("sem1");                                                         //musim unlinknut pre kazdy proces
    sem_unlink("mutex");  
    sem_unlink("semSanta");  
    sem_unlink("semDoor");  
    sem_unlink("semElfsDone"); 
    sem_unlink("semSobWaitforSanta"); 
    sem_unlink("semSobOrElf"); 
    sem_unlink("semSobCount"); 
    sem_unlink("semPrint"); 
    sem_unlink("semHitch"); 

}
bool Test(int argc, char* argv[]){
    if (argc != 5)                                  //musia byt 4
    {
        fprintf(stderr,"Nesprávny počet argumentov\n");
        return false;
    }
    for (int i = 1; i <= 4; i++)                    //vsetky musia byt cisla(nezaporne) netreba kontrolovat -
    {
        if (!isNumber(argv[i]))
        {
            fprintf(stderr,"Kazdy argument musi byt cislo\n");
            return false;
        }
    }
    if(1 > atoi(argv[1]) || atoi(argv[1]) > 999)    //kontrola 1. argument
    {
        fprintf(stderr,"Prvy argument musi byt 0<arg<1000\n");
        return false;
    }
    if(1 > atoi(argv[2]) || atoi(argv[2]) > 19)     //kontrola 2. argument
    {
        fprintf(stderr,"Druhy argument musi byt 0<arg<20\n");
        return false;
    }
    if(-1 > atoi(argv[3]) || atoi(argv[3]) > 1000)  //kontrola 3. argument
    {
        fprintf(stderr,"Treti argument musi byt 0<=arg<=1000\n");
        return false;
    }
    if(-1 > atoi(argv[4]) || atoi(argv[4]) > 1000)  //kontrola 4. argument
    {
        fprintf(stderr,"Stvrty argument musi byt 0<=arg<=1000\n");
        return false;
    }
    return true;       
}
bool isNumber(char* string){

    int count=0;
    while(string[count]!='\0')
    {
        if ('9' < string[count] || string[count] < '0')
        {
            return false;
        }
        count++;
    }
    return true;
}