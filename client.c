#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#define HOST "34.118.48.238"
#define PORT 8080

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);

    char* cmd = malloc(100*sizeof(char));
    char* my_cookie = malloc(300*sizeof(char));
    char* JWT = calloc(700,sizeof(char));
    int loggedin = 0;


    while(1){

        fgets(cmd, 100, stdin);
         
        strtok(cmd," \n");

        if(strcmp(cmd,"register") == 0){ //Daca comanda este register
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);
            char* username = malloc(50*sizeof(char)); 
            char* password = malloc(50*sizeof(char));
            char *url = "/api/v1/tema/auth/register"; // Caile pentru fiecare comanda sunt salvate local
            char *content_type = "application/json";
            
            JSON_Value *root = json_value_init_object();
            JSON_Object *user = json_value_get_object(root);  //Cream obiectul json cu ajutorul librariei parson.

            printf("Username=");
            scanf("%s",username);
            printf("Password=");
            scanf("%s",password);
            
            json_object_set_string(user,"username",username);
            json_object_set_string(user,"password",password);

            char* string = json_serialize_to_string_pretty(root);

            message = compute_post_request(HOST, url,content_type,&string,1,&my_cookie,JWT); //creeam mesajul de trimis serverului
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);

            char* my_err = strstr(response,"{\"error"); //verificam daca serverul intoarce vreo eroare
            if(stderr){
                printf("%s\n",my_err );
            }
            else
                printf("Inregistrarea a reusit!\n"); //in caz contrar, comanda a avut succes

            json_free_serialized_string(string); //elibeream memoria alocata
            json_value_free(root);
            free(username);
            free(password);
            close_connection(sockfd);
        }
    else

        if(strcmp(cmd,"login") == 0){ //verificam daca comanda este login
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);

            if(loggedin == 1){ //daca esti deja logat, iesim si afisam mesajul
                printf("Esti deja logat! Delogheaza-te mai intai!\n");
                continue;
            }

            char* username = calloc(50,sizeof(char)); 
            char* password = calloc(50,sizeof(char));
            char *url = "/api/v1/tema/auth/login";
            char *content_type = "application/json";

            JSON_Value *root = json_value_init_object();
            JSON_Object *user = json_value_get_object(root);

            printf("Username=");
            scanf("%s",username);
            printf("Password=");
            scanf("%s",password);

            json_object_set_string(user,"username",username);
            json_object_set_string(user,"password",password);

            char* string = json_serialize_to_string_pretty(root);

            message = compute_post_request(HOST, url,content_type,&string,1,&my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);
           
            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err );
                continue;
            }
                
            if(strlen(response) > 3){
                printf("Te-ai logat cu succes!\n");
                loggedin = 1;
                char *this_cookie = strstr(response,"Set-Cookie:"); // salvam cookie-ul de sesiune pentru demonstatiie viitoare
                strtok(this_cookie,"\r\n");
                this_cookie =strcpy(this_cookie,this_cookie+12);
                sprintf(my_cookie,"%s",this_cookie);
            }
            

            json_free_serialized_string(string);
            json_value_free(root);
            free(username);
            free(password);
            close_connection(sockfd);
        }
    else

        if(strcmp(cmd,"logout") == 0){

            if(loggedin == 0){
                printf("Nu esti logat!\n");
                continue;
            }

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);
            char *url = "/api/v1/tema/auth/logout";
            char *content_type = "application/json";

            message = compute_get_request(HOST,url,content_type,&my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);
            
            if(strlen(response) > 3){
                printf("Te-ai delogat cu succes!\n");
                loggedin = 0;
            }

            memset(JWT, 0, 700);
            memset(my_cookie, 0, 300);

            close_connection(sockfd);
        }

    else

        if(strcmp(cmd,"enter_library") == 0){
            
            if(loggedin == 0){
                printf("Trebuie sa fii logat ca sa accesezi biblioteca!\n");
                continue;
            }

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);
            char *url = "/api/v1/tema/library/access";
            char *content_type = "application/json";

            message = compute_get_request(HOST, url, content_type, &my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);
        
            if(strlen(response) > 3){
                char *this_cookie = strstr(response,"\"token\":"); //parsam mesajul de la server pentru a obtine tokenul JWT
                this_cookie = this_cookie + 9;
                char* token = NULL;
                token = strstr(this_cookie,"\"}");
                strncpy(JWT,this_cookie, token - this_cookie );
            }

            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err );
            }
            else
                printf("Ai accesat biblioteca cu succes!\n");

            close_connection(sockfd);
            
        }
    
    else

        if(strcmp(cmd,"get_books") == 0){

            if(loggedin == 0){
                printf("Trebuie sa fii logat ca sa accesezi biblioteca!\n");
                continue;
            }

            if(strlen(JWT) <= 0){
                printf("Trebuie sa accesezi biblioteca mai intai!\n");
                continue;
            }
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);

            char* url = "/api/v1/tema/library/books";
            char *content_type = "application/json";

            message = compute_get_request(HOST,url,content_type,&my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);

            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err );
            }
            else{
                char* books = strstr(response,"[");
                printf("%s\n",books );
            }
                

            close_connection(sockfd);
        }

    else

        if(strcmp(cmd,"get_book") == 0){

            if(loggedin == 0){
                printf("Trebuie sa fii logat ca sa accesezi biblioteca!\n");
                continue;
            }
            
            if(strlen(JWT) <= 0){
                printf("Trebuie sa accesezi biblioteca mai intai!\n");
                continue;
            }

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);

            char url[100] = "/api/v1/tema/library/books/";
            char *content_type = "application/json";
            char id[25455];

            printf("id:");
            scanf("%s",id);
            sprintf(url,"%s%s",url,id);

            message = compute_get_request(HOST,url,content_type,&my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);

            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err);
            }
            else{
                char* books = strstr(response,"[");
                printf("%s\n",books );
            }

        close_connection(sockfd);
        }
    else
        if(strcmp(cmd,"add_book") == 0){

            if(loggedin == 0){
                printf("Trebuie sa fii logat ca sa accesezi biblioteca!\n");
                continue;
            }
            
            if(strlen(JWT) <= 0){
                printf("Trebuie sa accesezi biblioteca mai intai!\n");
                continue;
            }

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);
            char* title = malloc(100*sizeof(char));
            char* author = malloc(100*sizeof(char));
            char* genre = malloc(100*sizeof(char));
            int page_count = 0;
            char* publisher = malloc(100*sizeof(char));

            char *url = "/api/v1/tema/library/books";
            char *content_type = "application/json";

            JSON_Value *root = json_value_init_object();
            JSON_Object *book = json_value_get_object(root);

            printf("Title:");scanf("%s",title);
            printf("Author:");scanf("%s",author);
            printf("Genre:");scanf("%s",genre);
            printf("Page_count:");scanf("%d",&page_count);
            printf("Publisher:");scanf("%s",publisher);

            json_object_set_string(book,"title",title);
            json_object_set_string(book,"author",author);
            json_object_set_string(book,"genre",genre);
            json_object_set_number(book,"page_count",page_count);
            json_object_set_string(book,"publisher",publisher);


            char* string = json_serialize_to_string_pretty(root); //creeam obiectul json pentru cartea de adaugat

            message = compute_post_request(HOST,url,content_type,&string,1,&my_cookie,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);

            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err);
            }
            else{
                printf("Cartea a fost adaugata cu succes!\n");
            }


            close_connection(sockfd);
        }
    else
        if(strcmp(cmd,"delete_book") == 0){

            if(loggedin == 0){
                printf("Trebuie sa fii logat ca sa accesezi biblioteca!\n");
                continue;
            }
            
            if(strlen(JWT) <= 0){
                printf("Trebuie sa accesezi biblioteca mai intai!\n");
                continue;
            }

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM,0);

            char url[100] = "/api/v1/tema/library/books/";
            char *content_type = "application/json";
            char id[25455];

            printf("id:");
            scanf("%s",id);
            sprintf(url,"%s%s",url,id);

            message = compute_delete_request(HOST,url,JWT);
            send_to_server(sockfd,message);
            response = receive_from_server(sockfd);

            char* my_err = strstr(response,"{\"error");
            if(my_err){
                printf("%s\n",my_err);
            }
            else{
                printf("Cartea a fost stearsa cu succes!\n");
            }
        }
    else

        if(strcmp(cmd,"exit") == 0){
            return 0;
        }
    }

    free(message);

    return 0;
}
