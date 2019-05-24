/*
 * proxy.c - ICS Web proxy
 *
 *
 */

#include "csapp.h"
#include <stdarg.h>
#include <sys/select.h>

/*
    echo function for echo server .
*/
void echo(int connfd)
{

    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio,buf,MAXLINE)) != 0){
        printf("server received %d bytes\n",(int) n);
        Rio_writen(connfd, buf, n);
    }
}

/*
 *	wrapper functions of myouwn
 */
ssize_t Rio_writen_w(int fd, void *usrbuf,size_t n);
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf,size_t n);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf,size_t maxlen);
ssize_t Open_clientfd_w(char *hostname,char *port);
/*
 * core code to achieve concurrency
 */
void *thread(void *vargp);
/*
 * core code of my proxy
 */
void doproxy(int connfd,struct sockaddr_in *clientaddr);
//void read_requesthdrs(rio_t *rp, int * size);
 /* Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, char *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, size_t size);


/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{

    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /*the same as _in but with 128 bytes*/
    char client_hostname[MAXLINE], client_port[MAXLINE];

    Signal(SIGPIPE,SIG_IGN);

    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE,
                               client_port, MAXLINE, 0);
        //printf("Connected to (%s, %s)\n",client_hostname,client_port);
        doproxy(connfd,(struct sockaddr_in *)&clientaddr);
       	Close(connfd);
    }
    exit(0);
}

/*
	void *thread
	thread main body
*/
void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	int tid = *(((int *)vargp)+1)
	printf("%d,%d\n",connfd,tid );
	Pthread_detach(pthread_self());
	Free(vargp);
	echo(connfd)

}
/* void doproxy(int connfd)
    proxy main body;
*/
void doproxy(int fd,struct sockaddr_in *clientaddr)
{
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE];
    char requestline[MAXLINE];
    rio_t r_client;
    size_t size = 0;
    int req_tmp = 0, req_content_length = 0;

    /* read Request line*/
    Rio_readinitb(&r_client, fd);
    req_tmp	= Rio_readlineb_w(&r_client, buf, MAXLINE);
    //-size += strlen(buf);
//    printf("------------------------\n");
 //   printf("%sreq_tmp:%d\n", buf,req_tmp);
    if(req_tmp <= 0 || (buf[strlen(buf)-1] != '\n' && (buf[strlen(buf)-2] != '\r'))) {
//    	printf("out\n");
    	return ;
    }
    strcpy(requestline,buf);
    sscanf(buf, "%s %s %s",method,uri,version);

    /*parse uri*/
    char hostname[MAXLINE],pathname[MAXLINE],port[MAXLINE];
    parse_uri(uri,hostname,pathname,port);

	/*establish server connection*/
    rio_t r_server;
    int serverfd = Open_clientfd_w(hostname,port);
    if(serverfd <= 0){
 //   	printf("out0\n");
    	return;
    }
    Rio_readinitb(&r_server, serverfd);

    /*send request line*/
    sprintf(requestline,"%s /%s %s\r\n",method,pathname,version);
    Rio_writen_w(serverfd,requestline,strlen(requestline));

	/*transport headers */
    int req_read = 0, req_write = 0;
    do{
        req_tmp = Rio_readlineb_w(&r_client,buf,MAXLINE);/*read new header line*/
        if(req_tmp <= 0) {
 //       	printf("out1\n");
			Close(serverfd);
        	return ;
        }
 //       printf("|-%s",buf);
        //-size += strlen(buf);
        req_write = Rio_writen_w(serverfd,buf,strlen(buf));    /*send to server*/
        if(req_write != strlen(buf)){
//        	printf("out2\n");
			Close(serverfd);
        	return ;
        }
        if(!strncasecmp(buf,"Content-Length",14))
        	req_content_length = atoi(buf +15);
    }while(strcmp(buf, "\r\n"));
    //printf("headers-transported\n");

    /* transport body*/
    memset(buf,0,sizeof(buf));
    if(req_content_length > 0)
    {
    	req_tmp = req_content_length;
    	//-size += req_content_length;
 //   	printf("req_content_length:%d\n",req_content_length);
    	while(req_tmp > 0){
    		req_read = Rio_readnb_w(&r_client,buf,1);
    		if(req_read <= 0){
    		//	printf("out3\n");
				Close(serverfd);
    			return ;
    		}
    //		printf("%c",buf[0]);
			req_write = Rio_writen_w(serverfd,buf,1); /*can't use sprintf because of possible '\0'*/
    		if(req_write != 1){
    		//	printf("out4\n");
    			Close(serverfd);
    			return ;
    		}
    		req_tmp --;
    	}
    }
    //printf("req transported\n");
    /*forward response headline and headers to client*/
    int res_tmp = 0,res_content_length = 0;
    do{
    	res_tmp = Rio_readlineb_w(&r_server,buf,MAXLINE);
    	size += res_tmp;
    	if(res_tmp <= 0) {
    		//printf("res_tmp:%d\n",res_tmp );
    		Close(serverfd);
    		return;
    	}
    	Rio_writen_w(fd,buf,strlen(buf));
    	//printf("|-%s",buf);
    	if(!strncasecmp(buf,"Content-Length",14)){
    		res_content_length = atoi(buf + 15);
    	}
    }while(strcmp(buf,"\r\n"));
    
    /*forward response body to client*/
    /*
    res_tmp = res_content_length;
    while(res_tmp > 0){
    	Rio_readnb_w(&r_server,buf,1);
    	Rio_writen_w(fd,buf,1);
    	res_tmp --;
    }*/ /* alternate approach which keep track of res_length*/
    int res_read = 0,res_write = 0;
	if(res_content_length > 0){
		size += res_content_length;
		res_tmp = res_content_length;
		//printf("res_content_length: %d\n",res_content_length);

		while(res_tmp >0){
			res_read = Rio_readnb_w(&r_server,buf,1);
			if(res_read <= 0 ){
				Close(serverfd);
				//printf("out5\n");
				return;
			}
			res_write = Rio_writen_w(fd,buf,1);
			if(res_write <= 0){
				Close(serverfd);
				return ;
			}
			res_tmp --; 
		}
	}
	

	//printf("safe close\n");
	Close(serverfd);

	/*write log*/
    char logstring[MAXLINE];
	format_log_entry(logstring,clientaddr,uri,size);
	printf("%s\n",logstring);

    return ;	

}

/*
 * wrapper funciton implementations	
 */
ssize_t Rio_writen_w(int fd, void *usrbuf,size_t n)
{
	ssize_t rc;
	if((rc = rio_writen(fd,usrbuf,n)) != n)
	{
		fprintf(stderr, "%luRio writen error %d: %s\n",rc,errno,strerror(errno));
	}
	return rc;
}
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf,size_t n)
{
	ssize_t rc;
	if((rc = rio_readnb(rp,usrbuf,n)) != n )
	{
		fprintf(stderr, "%luRio readnb error %d: %s\n",rc,errno,strerror(errno));

	}
	return rc;
}
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf,size_t maxlen){
	ssize_t rc;
	if((rc = rio_readlineb(rp,usrbuf,maxlen)) <= 0){  /* */
		fprintf(stderr, "%luRio readlineb error %d : %s\n",rc,errno,strerror(errno));
	}
	return rc;

}
ssize_t Open_clientfd_w(char *hostname, char *port)
{
	ssize_t rc;
	if((rc = open_clientfd(hostname,port)) <= 0)
	{
		fprintf(stderr, "%lu Open clientfd error %d:%s\n",rc,errno,strerror(errno));
	}
	return rc;
}
/*
    void read_requesthdrs;
    read and print request hders
*/
void read_requesthdrs(rio_t *r_client,int sfd,int *size){
  return;
}
/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    if (hostend == NULL)
        return -1;
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    if (*hostend == ':') {
        char *p = hostend + 1;
        while (isdigit(*p))
            *port++ = *p++;
        *port = '\0';
    } else {
        strcpy(port, "80");
    }

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;
        strcpy(pathname, pathbegin);
    }
    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), the number of bytes
 * from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, size_t size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 12, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %zu", time_str, a, b, c, d, uri, size);
}


