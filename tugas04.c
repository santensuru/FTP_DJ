// FTP server

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

int len_root = 20;
char dir_root[1024] = "/home/user/coba/FTP/";
char dir_home[1024] = "/";

int pre(char *ls) {
    int fd;
    char buf[2], msg[4096], *ptr;
    
    fd = open(ls, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    memset(msg, 0, sizeof(msg[4096]));
    
    do {
        fflush(stdin);
        int retval = read(fd, buf, sizeof(buf)-1);
        buf[retval] = '\0';
        strcat(msg, buf);
        //printf("%c", buf);
    } while (strstr(msg, "</SETTING>") == NULL);
    
    memset(dir_root, 0, sizeof(dir_root[1024]));
    
    if (ptr = strstr(msg, "<DIRECTORY>")) {
        sscanf(ptr, "<DIRECTORY> %s", dir_root);
    }
    len_root = strlen(dir_root);
    
    printf("OK");
    
    close(fd);
    return 3;
}

typedef struct data_listen {
    int sockcli_data;
    int sockfd_data;
    int clisize_data;
    struct sockaddr* cliaddr_data;
} data_listen;

void *data(void *ptr) {
    data_listen * baru = (data_listen *)ptr;
	listen((baru->sockfd_data), 5);
    (baru->sockcli_data) = accept((baru->sockfd_data), ((baru->cliaddr_data)) , &((baru->clisize_data)));
    printf("%d %d \n", (baru->sockcli_data), (baru->sockfd_data) );
    return (void *)(baru->sockcli_data);
}

void get_user(char *user, char *pass) {
    int fd;
    char buf[2], msg[4096], *ptr, *ptr2, *ptr3, temp[1024];
    
    fd = open("FTP_DJ.xml", O_RDONLY);
    
    if (fd < 0)
        exit(-1);
    
    do {
        fflush(stdin);
        int retval = read(fd, buf, sizeof(buf)-1);
        buf[retval] = '\0';
        strcat(msg, buf);
    } while (strstr(msg, "</USERS>") == NULL);
    
    if (ptr = strstr(msg, "<USERS>")) {
        if (ptr2 = strstr(ptr, user)) {
            if (ptr3 = strstr(ptr2, "<PASSWORD>")) {
                sscanf(ptr3, "<PASSWORD> %s", pass);
            }
            else {
                strcpy(pass, "\r\n");
            }
        }
        else {
            strcpy(pass, "\r\n");
        }
    }
    
    close(fd);
    
    return;
    
    /*
    if (strcmp(user, "djuned") == 0)
        strcpy(pass, "haha");
    else
        strcpy(pass, "\r\n");
    return;
    */
}

int list(int sockcli) { // char *ls
    DIR * d;
    //char * dir_name = "/home/user/coba/FTP/";
    char ls[512];
    //memset(ls, 0, sizeof(ls[4096]));
    /* Open the current directory. */

    d = opendir (dir_root);

    if (! d) {
        //fprintf (stderr, "Cannot open directory '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }
    while (1) {
        struct dirent * entry;
        struct stat fileStat;
        struct tm *gmt;
        
        char formatted_gmt[50];
        char temp[128];
        int status;
        
        struct passwd  *pwd;
        struct group   *grp;
        char temp_b[128];
        
        entry = readdir (d);
        if (! entry) {
            break;
        }
        
        //printf ("%s\n", entry->d_name);
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        strcpy(temp, dir_root);
        strcat(temp, entry->d_name);
        
        status =  stat(temp, &fileStat);
        if(status < 0) {
            perror(strerror(errno));
            exit(-1);
        }
        else {
            strcpy(ls, "");
            
            strcat(ls, ( S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            strcat(ls, (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            strcat(ls, (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            strcat(ls, (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            strcat(ls, (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            strcat(ls, (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            strcat(ls, (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            strcat(ls, (fileStat.st_mode & S_IROTH) ? "r" : "-");
            strcat(ls, (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            strcat(ls, (fileStat.st_mode & S_IXOTH) ? "x" : "-");
            
            /* Print out type, permissions, and number of links. */
            //printf("%10.10s", sperm (statbuf.st_mode));
            sprintf(temp_b, " %4d", fileStat.st_nlink);

            strcat(ls, temp_b);
            
            /* Print out owner's name if it is found using getpwuid(). */
            if ((pwd = getpwuid(fileStat.st_uid)) != NULL)
                sprintf(temp_b, " %-8.8s", pwd->pw_name);
            else
                sprintf(temp_b, " %-8d", fileStat.st_uid);
            
            strcat(ls, temp_b);
            
            /* Print out group name if it is found using getgrgid(). */
            if ((grp = getgrgid(fileStat.st_gid)) != NULL)
                sprintf(temp_b, " %-8.8s", grp->gr_name);
            else
                sprintf(temp_b, " %-8d", fileStat.st_gid);
            
            strcat(ls, temp_b);
            
            /* Print size of file. */
            sprintf(temp_b, " %9jd", (intmax_t)fileStat.st_size);
            
            strcat(ls, temp_b);
        
            strcat(ls, " ");
            
            /* Get localized date string. */        
            gmt = localtime(&(fileStat.st_ctime));
            //printf("%ld", fileStat.st_ctime);
        
            strftime(formatted_gmt, sizeof(formatted_gmt), "%b %d %R", gmt);
            strcat(ls, formatted_gmt);
        
            strcat(ls, " ");
        }
        
        strcat(ls, entry->d_name);
        strcat(ls, "\r\n");
        
        // send to recivier
        write(sockcli, ls, strlen(ls));
    }
    
    //// send to recivier
    //write(sockcli, ls, strlen(ls));
    
    /* Close the directory. */
    if (closedir (d)) {
        //fprintf (stderr, "Could not close '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }

    return 3;
}

int dir(char *ls) {
    DIR * d;
    char temp[4096];
    
    strcpy(temp, dir_root);
    int t = 0;
    
    while (strstr(ls, "../")) {
        int i;
        int x = strlen(ls);
        for (i=3; i<x; i++) {
            ls[i-3] = ls[i];
        }
        ls[i-3] = '\0';
        x = strlen(temp);
        if (x == len_root) {
            t = 1;
            break;
        }
        temp[x-1] = '\0';
        for (i=x-2; i>=0; i--) {
            if (temp[i] == '/') break;
            temp[i] = '\0';
        }
        t = 2;
    }
    
    if (t == 0) {
        //puts(temp);
        strcat(ls, "/");
        strcat(temp, ls);
    }
    else if (t == 2) {
        //puts(temp);
        strcat(temp, ls);
    }
    
    /* Open the current directory. */

    d = opendir (temp);

    if (! d) {
        //fprintf (stderr, "Cannot open directory '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }
    
    /* Close the directory. */
    if (closedir (d)) {
        //fprintf (stderr, "Could not close '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }
    
    memset(dir_root, 0, strlen(dir_root));
    strcpy(dir_root, temp);
    memset(dir_home, 0, strlen(dir_home));
    int i = len_root-1;
    int l = strlen(dir_root);
    for (i; i<l; i++) {
        dir_home[i-len_root+1] = dir_root[i];
    }
    dir_home[i] = '\0';
    printf("%s", dir_root);
    printf("\n%s\n", dir_home);

    return 3;
}

int download(char *name, int sockcli) {
    int fd, buf_size;
    char buf[4096];
    char str_name[128];
    strcpy(str_name, dir_root);
    strcat(str_name, name);
    
    fd = open(str_name, O_RDONLY);
    
    if (fd < 0)
        return -1;

    do {
        /* binaries */
        buf_size = read(fd, &buf[0], 4096);
        //buf[buf_size] = '\0';
        //puts(buf);
        if (buf_size > 0) {
            int s = write(sockcli, &buf[0], buf_size);
            //printf("%d\n", s);
            //printf("\n%d", buf_size);
            //fflush(stdout);
        }
    } while (buf_size > 0);

    close(fd);
    return 3;
}

int upload(char *name, int sockcli) {
    int fd, buf_size;
    char buf[1], msg[128];
    char str_name[128];
    strcpy(str_name, dir_root);
    strcat(str_name, name);
    
    fd = open(str_name, O_WRONLY | O_CREAT, S_IRWXU);
    
    if (fd < 0)
        return -1;
        /*
    do {
        buf_size = read(sockcli, buf, 4096);
        if (buf_size > 0) {
            int s = write(fd, buf, buf_size);
        }
    } while (buf_size > 0);*/
    
        //memset(msg, 0, sizeof(msg[4096]));
        strcpy(msg, "Enter file after this: ....\r\n");
        write(sockcli, msg, strlen(msg));
        
        //memset(msg, 0, sizeof(msg[4096]));
        
        int retval;
        do {
            fflush(stdin);
            /* binaries */
            retval = read(sockcli, &buf[0], 1);
            write(fd, &buf[0], retval);
            //printf("%d", retval);
            //buf[retval] = '\0';
            //memcpy((void *)&msg[0], (const void *)&buf[0], retval); //strcat(msg, buf);
        } while (retval > 0); //while (strstr(msg, "\r\n\r\n") == NULL);
        //write(fd, msg, strlen(msg)-2);

    close(fd);
    return 3;
}

int del(char *name) {
    char str_name[128];
    strcpy(str_name, dir_root);
    strcat(str_name, name);
    int s = remove(str_name);
    if (s < 0)
        return -1;
    return 3;
}

int main() {
    int q = pre("FTP_DJ.xml");
    if (q < 0) {
        printf("FTP_DJ.xml Not Found or Not in same directory\n\n");
        exit(-1);
    }
    
    int sockfd = 0, sockfd_data = 0, sockcli = 0, sockcli_data = 0;
    int retval = 0;
    struct sockaddr_in servaddr, servaddr_data, cliaddr, cliaddr_data;
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6060);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   
    
    retval = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    
    if (retval <0) {
        perror(strerror(errno));
        exit(-1);
    }
    
    printf("Server mengikat port 6060\n");
    retval = listen(sockfd, 5);
    printf("Server menunggu panggilan...\n");
    
    memset(&cliaddr, 0, sizeof(cliaddr));
    memset(&cliaddr_data, 0, sizeof(cliaddr_data));
    socklen_t clisize = sizeof(cliaddr), clisize_data = sizeof(cliaddr_data);
    //clisize = 0;
    //clisize_data = 0;
     
    sockcli = accept(sockfd, (struct sockaddr*)&cliaddr , &clisize);
    
    printf("Ada klien masuk dari %s %d\n", inet_ntoa(cliaddr.sin_addr), (int) ntohs(cliaddr.sin_port));
    
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cliaddr.sin_addr), str, INET_ADDRSTRLEN);
    printf("%s\n", str);
    
    struct sockaddr_in localAddress;
    socklen_t addressLength = sizeof(localAddress);
    getsockname(sockcli, (struct sockaddr*)&localAddress, &addressLength);
    char ip_active[16];
    strcpy(ip_active, inet_ntoa( localAddress.sin_addr));
    printf("local address: %s\n", ip_active);
    printf("local port: %d\n", (int) ntohs(localAddress.sin_port));
    
    if (sockcli < 0) {
        perror(strerror(errno));
        exit(-1);
    }
    
    // baca dan tulis pesan disini
    
    int flag = 1;
    int login = 0;
    int port = 0;
    int error = 0;
    
    char buf[2], msg[255], comment[255], pass[255], msg_send[4096], ack[5];
    
    data_listen *baru = (data_listen *) malloc( sizeof ( data_listen ) );
    
    // thread listen //
    void *status;
    
    pthread_t data_t;
    int data_i;
	pthread_mutex_t data_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
    
    sprintf(msg_send, "220-FTP_DJ Server version 0.0.2d beta\n\r220-written by Djuned Fernando Djusdek (djuned.ong@gmail.com)\n\r220 Please visit https://github.com/santensuru/FTP_DJ\n\r");
    //printf("%s", msg_send);
    write(sockcli, msg_send, strlen(msg_send));
    fflush(stdout);
    
    while (flag) {
        
        memset(msg, 0, sizeof(msg[1024]));
        do {
            fflush(stdin);
            retval = read(sockcli, buf, sizeof(buf)-1);
            buf[retval] = '\0';
            strcat(msg, buf);
        } while (strstr(msg, "\r\n") == NULL);
        
        //printf("%s", msg);
        
        if (strstr(msg, "USER") != NULL) {
            sscanf(msg, "USER %s", comment);
            get_user(comment, pass);
            //printf("331 Password required for %s\r\n", comment);
            sprintf(msg_send, "331 Password required for %s\r\n", comment);
            error = 0;
        }
        
        else if (strstr(msg, "PASS") != NULL) {
            sscanf(msg, "PASS %s", comment);
            if (strcmp(comment, pass) == 0 && error == 0) {
                sprintf(msg_send, "230 Logged on\r\n");
                login = 1;
                error = 0;
            }
            else if (error == 1) {
                sprintf(msg_send, "530 Login or password incorrect!\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "SYST") != NULL) {
            if (login && error == 0) {
                sprintf(msg_send, "215 UNIX Type: L8\r\n");
                error = 0;
            }
            else if (error == 1) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "PASV") != NULL) {
            if (login) {
                sockfd_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                memset(&servaddr_data, 0, sizeof(servaddr_data));
                
                servaddr_data.sin_family = AF_INET;
                servaddr_data.sin_port = htons(0);
                servaddr_data.sin_addr.s_addr = htonl(INADDR_ANY); 
                
                retval = bind(sockfd_data, (const struct sockaddr *)&servaddr_data, sizeof(servaddr_data));
                
                if (retval <0) {
                    perror(strerror(errno));
                    exit(-1);
                }
                
                struct sockaddr_in sin;
                socklen_t len = sizeof(sin);
                if (getsockname(sockfd_data, (struct sockaddr *)&sin, &len) == -1)
                    perror("getsockname");
                else
                    printf("port number %d\n", ntohs(sin.sin_port));
                
                int port_active = ntohs(sin.sin_port);
                int port_i[2];
                port_i[0] = port_active/256;
                port_i[1] = port_active%256;
                
                char ip_i[4][4];
                sscanf(ip_active, "%[^.].%[^.].%[^.].%s", ip_i[0], ip_i[1], ip_i[2], ip_i[3]);
                sprintf(msg_send, "200 Entering Passive Mode (%s,%s,%s,%s,%d,%d)\r\n", ip_i[0], ip_i[1], ip_i[2], ip_i[3], port_i[0], port_i[1]);
                //retval = listen(sockfd_data, 5);
                //sockcli_data = accept(sockfd_data, (struct sockaddr*)&cliaddr_data , &clisize_data);
                
                baru->sockfd_data = sockfd_data;
                baru->cliaddr_data = (struct sockaddr*)&cliaddr_data;
                baru->clisize_data;
                
                pthread_mutex_lock( &data_m );
        		data_i = pthread_create( &data_t, &attr, data, (void *)baru);
        		
        		pthread_mutex_unlock( &data_m );
        		
        		/*
                pthread_join(data_t, &status);
            
            	pthread_attr_destroy(&attr);
            	
            	sockcli_data = (int*)status;
        		printf("%d\n", (sockcli_data));
        		*/
        		
                error = 0;
                port = 1;
            }
        }
        
        else if (strstr(msg, "QUIT") != NULL) {
            sprintf(msg_send, "221 Goodbye\r\n");
            flag = 0;
            login = 0;
            error = 0;
        }
        
        else if (strstr(msg, "LIST") != NULL) {
            if (login && error == 0 && port) {
                sprintf(msg_send, "");
                //retval = listen(sockfd_data, 5);
                //sockcli_data = accept(sockfd_data, (struct sockaddr*)&cliaddr_data , &clisize_data);
                pthread_join(data_t, &status);
            	sockcli_data = (int*)status;
        		//printf("%d\n", (sockcli_data));
                if (sockcli_data < 0) {
                    sprintf(msg_send, "425 Can't open data connection\r\n");
                    error++;
                }
                else {
                    sprintf(msg_send, "150 Connection accepted\r\n");
                    int s = list(sockcli_data);
                    if (s < 0) {
                        sprintf(msg_send, "451 The server had trouble reading the directory from disk.\r\n");
                    }
                    else if (s == 3) {
                        strcat(msg_send, "226 Transfer OK.\r\n");
                    }
                    error = 0;
                }
                shutdown(sockcli_data, SHUT_WR);
                port = 0;
                //printf("%s", read(sockcli_data, ack, sizeof(ack)));
                //if (strstr(ack, "OK") != NULL) {
                    //close(sockcli_data);
                //}
            }
            else if (port == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "RETR") != NULL) {
            if (login && error == 0 && port) {
                sscanf(msg, "RETR %s", comment);
                sprintf(msg_send, "");
                //retval = listen(sockfd_data, 5);
                //sockcli_data = accept(sockfd_data, (struct sockaddr*)&cliaddr_data , &clisize_data);
                pthread_join(data_t, &status);
            	//pthread_attr_destroy(&attr);
            	sockcli_data = (int*)status;
            	printf("%d\n", sockcli_data);
                if (sockcli_data < 0) {
                    sprintf(msg_send, "425 Can't open data connection\r\n");
                    error++;
                }
                else {
                    sprintf(msg_send, "150 Connection accepted\r\n");
                    write(sockcli, msg_send, strlen(msg_send));
                    fflush(stdout);
                    int s = download(comment, sockcli_data);
                    if (s < 0) {
                        sprintf(msg_send, "551 The server had trouble reading the file from disk.\r\n");
                    }
                    else if (s == 3){
                        sprintf(msg_send, "226 The entire file was successfully written to the server's TCP buffers.\r\n");
                    }
                    error = 0;
                }
                shutdown(sockcli_data, SHUT_WR);
                port = 0;
                //read(sockcli_data, ack, sizeof(ack));
                //if (strstr(ack, "OK") != NULL)
                    //close(sockcli_data);
            }
            else if (port == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "STOR") != NULL) {
            if (login && error == 0 && port) {
                sscanf(msg, "STOR %s", comment);
                sprintf(msg_send, "");
                //retval = listen(sockfd_data, 5);
                //sockcli_data = accept(sockfd_data, (struct sockaddr*)&cliaddr_data , &clisize_data);
                pthread_join(data_t, &status);
            	//pthread_attr_destroy(&attr);
            	sockcli_data = (int*)status;
                if (sockcli_data < 0) {
                    sprintf(msg_send, "425 Can't open data connection\r\n");
                    error++;
                }
                else {
                    sprintf(msg_send, "150 Connection accepted\r\n");
                    write(sockcli, msg_send, strlen(msg_send));
                    fflush(stdout);
                    int s = upload(comment, sockcli_data);
                    if (s < 0) {
                        sprintf(msg_send, "551 The server had trouble writing the file from disk.\r\n");
                    }
                    else if (s == 3){
                        sprintf(msg_send, "226 The entire file was successfully written to the server's TCP buffers.\r\n");
                    }
                    error = 0;
                }
                shutdown(sockcli_data, SHUT_WR);
                port = 0;
                //read(sockcli_data, ack, sizeof(ack));
                //if (strstr(ack, "OK") != NULL)
                    //close(sockcli_data);
            }
            else if (port == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "DELE") != NULL) {
            if (login && error == 0 && port) {
                sscanf(msg, "DELE %s", comment);
                sprintf(msg_send, "");
                int s = del(comment);
                if (s < 0) {
                    sprintf(msg_send, "550 The server had trouble remove the file from disk.\r\n");
                }
                else{
                    sprintf(msg_send, "250 The file was successfully removed.\r\n");
                }
                error = 0;
            }
            else if (port == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "CWD") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "CWD %s", comment);
                sprintf(msg_send, "");
                int s = dir(comment);
                if (s < 0) {
                    sprintf(msg_send, "550 CWD failed. \"%s\": directory not found.\r\n", comment);
                }
                else{
                    sprintf(msg_send, "250 CWD successful. \"%s\" is current directory.\r\n", dir_home);
                }
                error = 0;
            }
            else if (login == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "PWD") != NULL) {
            if (login && error == 0) {
                sprintf(msg_send, "257 \"%s\" is current directory.\r\n", dir_home);
                error = 0;
            }
            else if (login == 0 && error == 0) {
                sprintf(msg_send, "530 Please log in with USER and PASS first.\r\n");
                error+=2;
            }
            else if (error == 1) {
                sprintf(msg_send, "501 Syntax error\r\n");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
            }
        }
        
        else if (strstr(msg, "HELP") != NULL) {
            sprintf(msg_send, "");
            strcat(msg_send, "214-The following commands are recognized:\r\n");
            strcat(msg_send, "   USER   PASS   QUIT   PASV   SYST   LIST   DELE   HELP\r\n");
            strcat(msg_send, "   RETR   STOR   CWD    PWD\r\n");
            strcat(msg_send, "214 Have a nice day.\r\n");
            error = 0;
        }
        
        else {
            sprintf(msg_send, "500 Syntax error, command unrecognized.\r\n");
        }

        write(sockcli, msg_send, strlen(msg_send));
        fflush(stdout);
    }
    
    /*
    char msg[20] ="Selamat datang kawan";
    retval = write(sockcli,msg,strlen(msg));
    printf("selesai kirim pesan\n");
    */
    
    close(sockcli);
    close(sockfd);
    
    return 0;
}
 
