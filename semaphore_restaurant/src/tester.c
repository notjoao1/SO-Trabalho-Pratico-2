#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "probConst.h"

#define LINEMAXSIZE 300
#define LIMIT TABLESIZE+2   // only nead to read columns of chef, waiter and client



int errorHappened(FILE *fp,int counter,int oldState,int newState){
    if(counter > 1)
        fprintf(stderr,"Error on client %d\nChange of state %d -> %d can't happen\n",counter-2,oldState,newState);
    else
        fprintf(stderr,"Error on %s\nChange of state %d -> %d can't happen\n",counter==1 ? "waiter" : "chefe",oldState,newState);
    
    fclose(fp);
    return EXIT_FAILURE;
}

int main(int argc, char const *argv[])
{
    FILE *fp = NULL;
    char *line = (char *) malloc(LINEMAXSIZE * sizeof(char));
    char *token;
    const char *delimeters = " ";
    int states[LIMIT];                                      // first 2 indexes are client and waiter
    int newState;
    int first = -1;
    int last = -1;
    int counter = 0;
    int arrivedCounter = 0;
    
    if( argc != 2 )
    {
        printf("USAGE: %s fileName\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* Open the file provided as argument */
    errno = 0;
    fp = fopen(argv[1], "r");
    if( fp == NULL )
    {
        perror ("Error opening file!");
        return EXIT_FAILURE;
    }
                     

    // ignorar primeiras 3 linhas do file (cabeçalho)
    if(fgets(line, LINEMAXSIZE, fp) == NULL){
        return errorHappened(fp,0,0,0);
    }
    if(fgets(line, LINEMAXSIZE, fp) == NULL){
        return errorHappened(fp,0,0,0);
    }
    if(fgets(line, LINEMAXSIZE, fp) == NULL){
        return errorHappened(fp,0,0,0);
    }
    

    if (fgets(line, LINEMAXSIZE, fp) != NULL)           // inicialização do estados
    {
        token = strtok(line, delimeters);
        while( token != NULL) {
            states[counter++]=atoi(token);

            if(counter == LIMIT){
                counter=0; 
                break;
            }

            token = strtok(NULL, delimeters);
        }
    }


    while( fgets(line, LINEMAXSIZE, fp) != NULL )
    {
        token=strtok(line,delimeters);
        while (token!=NULL)
        {

            if(states[counter]==(newState=atoi(token)))             // se não mudar o estado segue
                goto readNextState;

            if(counter > 1){                                        // se for cliente
                switch (states[counter])
                {
                    case INIT:
                        if(++arrivedCounter==TABLESIZE){
                            if(newState!=WAIT_FOR_FOOD)
                                return errorHappened(fp,counter,states[counter],newState);
                            last=counter;
                        }else{
                            if(newState!=WAIT_FOR_FRIENDS)
                                return errorHappened(fp,counter,states[counter],newState);
                            if(first==-1)
                                first=counter;
                        }
                        break;
                    
                    case WAIT_FOR_FRIENDS:
                        if(counter!=first){
                            if(newState!=WAIT_FOR_FOOD)
                                return errorHappened(fp,counter,states[counter],newState);
                        }else if(newState!=FOOD_REQUEST)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case FOOD_REQUEST:
                        if(first!=counter || newState!=WAIT_FOR_FOOD)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case WAIT_FOR_FOOD:
                        if(newState!=EAT)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case EAT:
                        if(newState != WAIT_FOR_OTHERS)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case WAIT_FOR_OTHERS:
                        if(counter!=last){
                            if(newState!= FINISHED)
                                return errorHappened(fp,counter,states[counter],newState);
                        }else if(newState!=WAIT_FOR_BILL)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case WAIT_FOR_BILL:
                        if(last!=counter || newState!=FINISHED)
                            return errorHappened(fp,counter,states[counter],newState);
                        break; 
                    
                    case FINISHED:
                        break;
                    
                    default:
                        return errorHappened(fp,counter,states[counter],newState);
                }

            }else if(counter == 1){                                 // se for waiter
                switch (states[counter])
                {
                    case WAIT_FOR_REQUEST:
                        if(newState==INFORM_CHEF){
                            ;
                        }else if(newState==TAKE_TO_TABLE){
                            for(int i=2; i<LIMIT;i++)               // check if all clients are in state WAIT_FOR_OTHERS or WAIT_FOR_BILL
                                if(states[i]!=WAIT_FOR_FOOD)
                                    return errorHappened(fp,counter,states[counter],newState);                            
                        }else if(newState!=RECEIVE_PAYMENT)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;                
                    case INFORM_CHEF:
                        if(newState!=WAIT_FOR_REQUEST)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case TAKE_TO_TABLE:
                        if(newState!=WAIT_FOR_REQUEST)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    
                    case RECEIVE_PAYMENT:
                        break;
                    
                    default:
                        return errorHappened(fp,counter,states[counter],newState);
                }
                

            }else{                                                  // se for chefe
                switch (states[counter])
                    {
                    case WAIT_FOR_ORDER:
                        if(newState!=COOK)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    case COOK:
                        if(newState!=REST)
                            return errorHappened(fp,counter,states[counter],newState);
                        break;
                    case REST:
                        break;
                    default:
                        return errorHappened(fp,counter,states[counter],newState);
                    }
            }
            
            states[counter] = newState;

readNextState:   
            if(++counter == LIMIT){
                counter=0; 
                break;
            }

            token = strtok(NULL, delimeters);
        }

    }


    puts("\t\t\t\tProgram ran smoothly");

    fclose(fp);

    return EXIT_SUCCESS;
}

// To test
// clear; cd ../src/; make all; ipcrm -a; cd ../run; ./test.sh 1