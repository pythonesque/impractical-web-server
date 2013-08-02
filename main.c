#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTEN_PORT 1024

void setnonblocking(int sock)
{
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	return;
}

inline void
set_socket_no_linger (int sock)
{
  static const struct linger linger = { .l_onoff = 1, .l_linger = 0 };
  // static const int reuse_addr = 1;
  // setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
  //    sizeof(reuse_addr));
  
  if (setsockopt (sock, SOL_SOCKET, SO_LINGER, &linger, (socklen_t) sizeof (linger)) < 0)
  {
    perror ("setsockopt");
    exit (EXIT_FAILURE);
  }
  
  printf ("Hrm!\n");
  
  // setnonblocking (sock);
  
  // if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof (reuse_addr)) < 0)
  // {
  //   perror ("setsockopt");
  //   exit (EXIT_FAILURE);
  // }
}

int
make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in6 name;
  // struct linger linger;
  
  /* Create the socket. */
  sock = socket (PF_INET6, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    exit (EXIT_FAILURE);
  }
  
  /* Make sure that when socket is closed, it does not continue to try to transmit data. */
  // linger.l_onoff = 0;
  // linger.l_linger = 0;
  // setsockopt (sock, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
  // set_socket_no_linger (sock);
  
  
  /* Give the socket a name. */
  name.sin6_family = AF_INET6;
  name.sin6_flowinfo = 0; /* Not implemented according to GNU documentation, but still apparently must be set to 0. */
  name.sin6_port = htons (port);
  name.sin6_addr = in6addr_loopback;
  
  if (bind (sock, (struct sockaddr *) &name, sizeof(name)) < 0)
  {
    perror ("bind");
    exit (EXIT_FAILURE);
  }
  
  return sock;
}

void
destroy_socket (int sock)
{ 
  // if (shutdown(sock, 2) < 0)
  // {
  //   perror ("shutdown");
  //   // exit (EXIT_FAILURE);
  // }
  
  set_socket_no_linger (sock);
  
  while (close (sock) < 0)
  {
    if (errno != EINTR)
    {
      perror ("close");
      exit (EXIT_FAILURE);
    }
  }
}

int
read_from_client (int sock)
{
  printf ("Reading from client!\n");
  return -1;
}

int
wait_socket (int sock, fd_set *active_fd_set, time_t seconds)
{
  int new_sock;
  socklen_t size;
  fd_set read_fd_set;
  int sock_index;
  struct timeval timeout;
  struct sockaddr_in6 clientname;
  
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;
  
  read_fd_set = *active_fd_set;
  while (1)
  {
    switch (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout))
    {
      case -1:
        /* Error */
        if (errno == EINTR)
        {
          /* Interrupt, resume wait with new timeout value */
          perror ("select[interrupt]");
          continue;
        }
        /* Fatal error */
        perror ("select");
        exit (EXIT_FAILURE);
      case 0:
        /* Timeout */
        printf("...\n");
        return -1;
      default:
        /* Success--time to examine pending sockets */
        for (sock_index = 0; sock_index < FD_SETSIZE; ++sock_index)
        {
          if (FD_ISSET (sock_index, &read_fd_set))
          {
            if (sock_index == sock)
            {
              /* New connection! */
              size = sizeof(clientname);
              new_sock = accept (sock, (struct sockaddr *) &clientname, &size);
              if (new_sock < 0)
              {
                perror ("accept");
                exit (EXIT_FAILURE);
              }
              
              // set_socket_no_linger (new_sock);
              
              FD_SET (new_sock, active_fd_set);
            }
            else
            {
              /* Old connection, new data */
              if (read_from_client (sock_index) < 0)
              {
                /* Kill the socket--it's done */
                destroy_socket (sock_index);
                FD_CLR (sock_index, active_fd_set);
              }
              
            }
          }
        }
        return 1;
    }
  }
}

int main (void)
{
  int sock;
  int sock_index;
  fd_set active_fd_set;
  
  sock = make_socket(LISTEN_PORT);
  
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);
  
  if (listen (sock, 1) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }
  
  if (wait_socket (sock, &active_fd_set, 1) >= 0)
  {
    printf("Received input!\n");
  }
  else
  {
    printf("No input received\n");
  }
  
  for (sock_index = 0; sock_index < FD_SETSIZE; ++sock_index)
  {
    if (FD_ISSET (sock_index, &active_fd_set))
    {
      printf("%d\n", sock_index);
      // if (sock != sock_index)
      // {
      //   printf("  Shutting down...\n");
      //   if (shutdown(sock_index, 0) < 0)
      //   {
      //     perror ("shutdown");
      //     exit (EXIT_FAILURE);
      //   }
      // }
      destroy_socket(sock_index);
      FD_CLR(sock_index, &active_fd_set);
    }
  }
  
  exit (EXIT_SUCCESS);
}