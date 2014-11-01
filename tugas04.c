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

#define TRUE   1
#define FALSE  0

int len_root = 20;
char dir_root[1024] = "/home/user/coba/FTP/";
//char dir_now[1024] = "/home/user/coba/FTP/";
//char dir_home[1024] = "/";

int pre(char *ls) {
    int fd;
    char buf[2], msg[4096], *ptr;
    
    fd = open(ls, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    memset(msg, 0, strlen(msg));
    
    do {
        fflush(stdin);
        int retval = read(fd, buf, 1);
        buf[retval] = '\0';
        strcat(msg, buf);
        //printf("%c", buf);
    } while (strstr(msg, "</SETTING>") == NULL);
    
    memset(dir_root, 0, strlen(dir_root));
    
    if (ptr = strstr(msg, "<DIRECTORY>")) {
        sscanf(ptr, "<DIRECTORY> %s", dir_root);
    }
    len_root = strlen(dir_root);
    //strcpy(dir_now, dir_root);
    
    printf("OK");
    //puts(dir_now);
    
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
        int retval = read(fd, buf, 1);
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

int list(int sockcli, char *dir_now) { // char *ls
    DIR * d;
    //char * dir_name = "/home/user/coba/FTP/";
    char ls[512];
    //memset(ls, 0, sizeof(ls[4096]));
    /* Open the current directory. */
    puts(dir_now);

    d = opendir (dir_now);

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
        
        strcpy(temp, dir_now);
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

int n_list(int sockcli, char *dir_now) { // char *ls
    DIR * d;
    //char * dir_name = "/home/user/coba/FTP/";
    char ls[512];
    //memset(ls, 0, sizeof(ls[4096]));
    /* Open the current directory. */
    puts(dir_now);

    d = opendir (dir_now);

    if (! d) {
        //fprintf (stderr, "Cannot open directory '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }
    while (1) {
        struct dirent * entry;
        
        strcpy(ls, "");
        
        entry = readdir (d);
        if (! entry) {
            break;
        }
        
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        strcat(ls, entry->d_name);
        strcat(ls, "\r\n");
        
        // send to recivier
        write(sockcli, ls, strlen(ls));
    }
    
    /* Close the directory. */
    if (closedir (d)) {
        //fprintf (stderr, "Could not close '%s': %s\n",
        //         dir_name, strerror (errno));
        //exit (EXIT_FAILURE);
        return -1;
    }

    return 3;
}

int dir(char *ls, char *dir_now, char *dir_home) {
    DIR * d;
    char temp[4096];
    bzero(temp, 4096);
    memset(dir_home, 0, strlen(dir_home));
    
    printf("ls: ");puts(ls);
    
    if (ls[0] == '/' && strlen(ls) == 1) {
        strcpy(dir_now, dir_root);
        strcpy(dir_home, "/");
        return 3;
    }
    
    else if (ls[0] == '/') {
        strcpy(temp, dir_root);
        //puts(temp);
        if (temp[strlen(temp)-1] == '/') {
            temp[strlen(temp)-1] = '\0';
        }
        if (ls[strlen(ls)-1] != '/') {
            ls[strlen(ls)] = '/';
            ls[strlen(ls)] = '\0';
        }
        strcat(temp, ls);
        
        dir_home[strlen(dir_home)-1] = '\0';
        strcpy(dir_home, ls);
        //puts(temp);
        goto ll;
    }
    
    //puts(ls);
    
    if (ls[0] != '/') {
        char t[128];
        strcpy(t, "/");
        strcat(t, ls);
        strcpy(ls, t);
    }
    
    strcpy(temp, dir_now);
    //printf("coba --> ");puts(temp);
    if (ls[strlen(ls)-1] != '/')
        strcat(ls, "/");
    int t = 0;
    
    while (1) {
        puts(temp);
        int z = strlen(temp) - 1;
        if (temp[z] == '/') {
            temp[z] = '\0';
        }
        else {
            break;
        }
    }
    
    temp[strlen(temp)] = '/';
    temp[strlen(temp)] = '\0';
    puts(temp);
    
    while (strstr(ls, "../")) {
        puts(ls);
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
        //strcat(ls, "/");
        if (ls[0] == '/') {
            temp[strlen(temp)-1] = '\0';
        }
        strcat(temp, ls);
    }
    else if (t == 2) {
        //puts(temp);
        if (ls[0] == '/') {
            temp[strlen(temp)-1] = '\0';
        }
        strcat(temp, ls);
    }
    
    //printf("coba --> ");puts(temp);
    
    ll:
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
    
    memset(dir_now, 0, strlen(dir_now));
    strcpy(dir_now, temp);
    memset(dir_home, 0, strlen(dir_home));
    int i = len_root-1;
    int l = strlen(dir_now);
    for (i; i<l; i++) {
        dir_home[i-len_root+1] = dir_now[i];
    }
    dir_home[i] = '\0';
    printf("%s", dir_now);
    printf("\nhome: %s\n", dir_home);
    //puts(dir_now);

    return 3;
}

int download(char *name, int sockcli, char *dir_now) {
    int fd, buf_size;
    char buf[4096];
    char str_name[128];
    strcpy(str_name, dir_now);
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

int upload(char *name, int sockcli, char *dir_now) {
    int fd, buf_size;
    char buf[1], msg[128];
    char str_name[128];
    strcpy(str_name, dir_now);
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
        //strcpy(msg, "Enter file after this: ....\r\n");
        //write(sockcli, msg, strlen(msg));
        
        //memset(msg, 0, sizeof(msg[4096]));
        
        int retval;
        do {
            fflush(stdin);
            /* binaries */
            retval = read(sockcli, buf, 1);
            write(fd, &buf[0], retval);
            //printf("%d", retval);
            //buf[retval] = '\0';
            //memcpy((void *)&msg[0], (const void *)&buf[0], retval); //strcat(msg, buf);
        } while (retval > 0); //while (strstr(msg, "\r\n\r\n") == NULL);
        //write(fd, msg, strlen(msg)-2);

    close(fd);
    return 3;
}

int del(char *name, char *dir_now) {
    char str_name[128];
    strcpy(str_name, dir_now);
    strcat(str_name, name);
    int s = remove(str_name);
    if (s < 0)
        return -1;
    return 3;
}

int dir_make(char *ls, char *dir_now) {
    char str_name[128];
    strcpy(str_name, dir_now);
    strcat(str_name, ls);
    int s = mkdir(str_name, 0755);
    if (s < 0)
        return -1;
    return 3;
}

int dir_del(char *ls, char *dir_now) {
    char str_name[128];
    strcpy(str_name, dir_now);
    strcat(str_name, ls);
    int s = rmdir(str_name);
    if (s < 0)
        return -1;
    return 3;
}

int exis(char *ls, char *dir_now) {
    char str_name[128];
    strcpy(str_name, dir_now);
    strcat(str_name, ls);
    int s = access( str_name, F_OK );
    if (s < 0)
        return -1;
    return 3;
}

int rn_to(char *to, char *ls, char *dir_now) {
    char str_name_src[128];
    char str_name_dst[128];
    strcpy(str_name_src, dir_now);
    strcat(str_name_src, ls);
    strcpy(str_name_dst, dir_root);
    str_name_dst[len_root-1] = '\0';
    strcat(str_name_dst, to);
    int s = rename(str_name_src, str_name_dst);
    //puts(str_name_src);
    //puts(str_name_dst);
    if (s < 0) {
        //perror(strerror(errno));
        return -1;
    }
    return 3;
}

int size(char *ls, char *dir_now, char *to) {
    char str_name[128];
    strcpy(str_name, dir_now);
    strcat(str_name, ls);
    struct stat fileStat;
    char buffer [33];
    
    int s =  stat(str_name, &fileStat);
    if(s < 0) {
        perror(strerror(errno));
        return -1;
    }
    else {
        bzero(to, strlen(to));
        int i=0;
        intmax_t temp_i = (intmax_t)fileStat.st_size;
        //printf("%d\n", temp_i);
        while (temp_i) {
            to[i] = (temp_i % 10) + '0';
            temp_i /= 10;
            i++;
        }
        //puts(to);
        int x = strlen(to);
        char buf[1];
        for (i=0; i<x/2; i++) {
            buf[0] = to[i];
            to[i] = to[x-i-1];
            to[x-i-1] = buf[0];
        }
        //puts(to);
    }
    return 3;
}

typedef struct haha {
    int sockcli;
    char ip_active[16];
} haha;

void *acc(void *ptr) {
    haha * handler = (haha *)ptr;
    
    char dir_now[1024] = "/home/user/coba/FTP/";
    strcpy(dir_now, dir_root);
    char dir_home[1024] = "/";
    
    printf("%d", handler->sockcli);
    int sockfd_data = 0, sockcli_data = 0;
    int retval = 0;
    struct sockaddr_in servaddr_data, cliaddr_data;
    
    memset(&cliaddr_data, 0, sizeof(cliaddr_data));
    socklen_t clisize_data = sizeof(cliaddr_data);
    
    int flag = 1;
    int login = 0;
    int port = 0;
    int error = 0;
    
    char buf[2], msg[255], comment[255], pass[255], msg_send[4096], ack[5];
    bzero(pass, 255);
    
    char version[128], user[128];
    strcpy(version, "0.0.5e beta");
    strcpy(user, "anonymously");
    
    data_listen *baru = (data_listen *) malloc( sizeof ( data_listen ) );
    
    // thread listen //
    void *status;
    
    pthread_t data_t;
    int data_i;
	pthread_mutex_t data_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
    
    sprintf(msg_send, "220-FTP_DJ Server version %s\n\r220-written by Djuned Fernando Djusdek (djuned.ong@gmail.com)\n\r220 Please visit https://github.com/santensuru/FTP_DJ\n\r", version);
    //printf("%s", msg_send);
    write(handler->sockcli, msg_send, strlen(msg_send));
    fflush(stdout);
    
    while (flag) {
        //memset(msg, 0, strlen(msg));
        bzero(msg, 255);
        bzero(comment, 255);
        bzero(msg_send, 4096);
        do {
            fflush(stdin);
            retval = read(handler->sockcli, buf, 1);
            buf[retval] = '\0';
            strcat(msg, buf);
            bzero(buf, 2);
        } while (strstr(msg, "\r\n") == NULL);
        
        int i;
        for (i=0; i<4; i++) {
            if (msg[i] >= 'a' && msg[i] <= 'z') {
                msg[i] = msg[i] - 'a' + 'A';
            }
        }
        
        //printf("%s", msg);
        
        if (strstr(msg, "USER") != NULL) {
            sscanf(msg, "USER %s", comment);
            strcpy(user, comment);
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
                strcpy(user, "anonymously");
                error++;
            }
            else {
                sprintf(msg_send, "503 Bad sequence of command\r\n");
                strcpy(user, "anonymously");
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
                sscanf(handler->ip_active, "%[^.].%[^.].%[^.].%s", ip_i[0], ip_i[1], ip_i[2], ip_i[3]);
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
                    int s = list(sockcli_data, dir_now);
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
                sscanf(msg, "RETR %[^\r\n]", comment);
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
                    write(handler->sockcli, msg_send, strlen(msg_send));
                    fflush(stdout);
                    int s = download(comment, sockcli_data, dir_now);
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
                sscanf(msg, "STOR %[^\r\n]", comment);
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
                    write(handler->sockcli, msg_send, strlen(msg_send));
                    fflush(stdout);
                    int s = upload(comment, sockcli_data, dir_now);
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
            if (login && error == 0) {
                sscanf(msg, "DELE %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = del(comment, dir_now);
                if (s < 0) {
                    sprintf(msg_send, "550 The server had trouble remove the file from disk.\r\n");
                }
                else{
                    sprintf(msg_send, "250 The file was successfully removed.\r\n");
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
        
        else if (strstr(msg, "CWD") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "CWD %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = dir(comment, dir_now, dir_home);
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
        
        else if (strstr(msg, "TYPE") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "TYPE %[^\r\n]", comment);
                printf("%s\n", comment);
                if (strcmp(comment, "L 8") == 0) {
                    sprintf(msg_send, "200 TYPE set to L 8\r\n");
                }
                else if (strcmp(comment, "I") == 0) {
                    sprintf(msg_send, "200 TYPE set to I\r\n");
                }
                else if (strcmp(comment, "A") == 0) {
                    sprintf(msg_send, "200 TYPE set to A\r\n");
                }
                else{
                    sprintf(msg_send, "501 Unsupported type. Supported types are A, I, L 8\r\n");
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
        
        else if (strstr(msg, "MKD") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "MKD %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = dir_make(comment, dir_now);
                if (s < 0) {
                    sprintf(msg_send, "550 \"%s%s\" creation failed\r\n", dir_home, comment);
                }
                else{
                    sprintf(msg_send, "257 \"%s%s\" create succesfully\r\n", dir_home, comment);
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
        
        else if (strstr(msg, "RMD") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "RMD %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = dir_del(comment, dir_now);
                if (s < 0) {
                    sprintf(msg_send, "550 Directory deleted failed\r\n");
                }
                else{
                    sprintf(msg_send, "250 Directory deleted successfully\r\n");
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
        
        else if (strstr(msg, "RNFR") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "RNFR %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = exis(comment, dir_now);
                if (s < 0) {
                    sprintf(msg_send, "550 File doesn't exists\r\n");
                }
                else{
                    sprintf(msg_send, "350 File exists, ready for destination name.\r\n");
                    strcpy(pass, comment);
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
        
        else if (strstr(msg, "RNTO") != NULL) {
            if (login && error == 0) {
                sscanf(msg, "RNTO %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = rn_to(comment, pass, dir_now);
                if (s < 0) {
                    sprintf(msg_send, "550 file can't renamed\r\n");
                }
                else{
                    sprintf(msg_send, "250 file renamed successfully\r\n");
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
        
        else if (strstr(msg, "SIZE") != NULL) {
            if (login && error == 0 && port) {
                sscanf(msg, "SIZE %[^\r\n]", comment);
                sprintf(msg_send, "");
                int s = size(comment, dir_now, pass);
                if (s < 0) {
                    sprintf(msg_send, "550 File not found\r\n");
                }
                else{
                    sprintf(msg_send, "213 %s\r\n", pass);
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
        
        else if (strstr(msg, "FEAT") != NULL) {
            sprintf(msg_send, "");
            strcat(msg_send, "211-Features:\r\n");
            strcat(msg_send, " SIZE\r\n");
            strcat(msg_send, "211 End\r\n");
            error = 0;
        }
        
        else if (strstr(msg, "NLST") != NULL) {
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
                    int s = n_list(sockcli_data, dir_now);
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
        
        else if (strstr(msg, "CDUP") != NULL) {
            if (login && error == 0) {
                strcpy(comment, "..");
                sprintf(msg_send, "");
                int s = dir(comment, dir_now, dir_home);
                if (s < 0) {
                    sprintf(msg_send, "550 CDUP Failed.\r\n");
                }
                else{
                    sprintf(msg_send, "200 CDUP successful. \"%s\" is current directory.\r\n", dir_home);
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
        
        else if (strstr(msg, "NOOP") != NULL) {
            sprintf(msg_send, "");
            strcat(msg_send, "200 OK\r\n");
            error = 0;
        }
        
        else if (strstr(msg, "STAT") != NULL) {
            char temp[128];
            sprintf(msg_send, "");
            strcat(msg_send, "211-ftp.heaven.af.mil FTP server status:\r\n");
            sprintf(temp, "   Version %s\r\n", version);
            strcat(msg_send, temp);
            sprintf(temp, "   Connected to heaven.af.mil (%s)\r\n", handler->ip_active);
            strcat(msg_send, temp);
            sprintf(temp, "   Logged in %s\r\n", user);
            strcat(msg_send, temp);
            strcat(msg_send, "   TYPE: ASCII, FORM: Nonprint; STRUcture: File; transfer MODE: Stream\r\n");
            strcat(msg_send, "   No data connection\r\n");
            strcat(msg_send, "211 End of status\r\n");
            error = 0;
        }
        
        else if (strstr(msg, "HELP") != NULL) {
            sscanf(msg, "HELP %s", comment);
            for (i=0; i<4; i++) {
                if (comment[i] >= 'a' && comment[i] <= 'z') {
                    comment[i] = comment[i] - 'a' + 'A';
                }
            }
            //puts(comment);
            sprintf(msg_send, "");
            strcat(msg_send, "214-The following commands are recognized:\r\n");
            strcat(msg_send, "   USER   PASS   QUIT   PASV   SYST   LIST   DELE   HELP\r\n");
            strcat(msg_send, "   RETR   STOR   CWD    PWD    TYPE   MKD    RMD    RNFR\r\n");
            strcat(msg_send, "   RNTO   SIZE   FEAT   NLST   CDUP   NOOP   STAT\r\n");
            strcat(msg_send, "214 Have a nice day.\r\n");
            if (strcmp(comment, "") != 0 ) {
                if (strstr(msg_send, comment) != NULL) {
                    sprintf(msg_send, "214 Command %s is supported by FTP_DJ\r\n", comment);
                }
                else {
                    sprintf(msg_send, "502 Command %s is not recognized or supported by FTP_DJ\r\n", comment);
                }
            }
            error = 0;
        }
        
        else {
            sprintf(msg_send, "500 Syntax error, command unrecognized.\r\n");
        }

        write(handler->sockcli, msg_send, strlen(msg_send));
        fflush(stdout);
    }
    
    /*
    char msg[20] ="Selamat datang kawan";
    retval = write(sockcli,msg,strlen(msg));
    printf("selesai kirim pesan\n");
    */
    
    close(handler->sockcli);
        
    return;
}

int main() {
    int q = pre("FTP_DJ.xml");
    if (q < 0) {
        printf("FTP_DJ.xml Not Found or Not in same directory\n\n");
        exit(-1);
    }
    
    int sockfd = 0, sockcli = 0;
    int retval = 0;
    struct sockaddr_in servaddr, cliaddr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int opt=TRUE;        /* option is to be on/TRUE or off/FALSE */
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt));
    
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
    socklen_t clisize = sizeof(cliaddr);
    //clisize = 0;
    //clisize_data = 0;
    
    // thread acc //
    void *join;
    
    pthread_t acc_t;
    int acc_i;
	pthread_mutex_t acc_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
	
    while(sockcli = accept(sockfd, (struct sockaddr*)&cliaddr , &clisize)) {
		
	    haha *handler = (haha *) malloc( sizeof ( haha ) );
	    printf("sockcli --> %d\n", sockcli);
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
	    
	    handler->sockcli = sockcli;
	    printf("%d\n", handler->sockcli);
	    strcpy(handler->ip_active, ip_active);
    	
	    pthread_mutex_lock( &acc_m );
		acc_i = pthread_create( &acc_t, &attr, acc, (void *)handler);
		
		pthread_mutex_unlock( &acc_m );
		
		//pthread_join(acc_t, &join);
	}
	
	close(sockfd);
	
	return 0;
}
 
