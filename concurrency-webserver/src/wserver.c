#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "pthread.h"

#define MAXBUF (50)

char default_root[] = ".";

int conn_fd_buffer[MAXBUF];
int fill_ptr = 0;
int use_ptr = 0;
int conn_fd_counter = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void put_conn_fd(int conn_fd) {
    conn_fd_buffer[fill_ptr] = conn_fd;
    fill_ptr = (fill_ptr + 1) % MAXBUF;
    conn_fd_counter++;
}

int get_conn_fd() {
    if (use_ptr == fill_ptr) {
        return -1; // buffer is empty
    }
    int fd = conn_fd_buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAXBUF;
    conn_fd_counter--;
    return fd;
}

void *request_worker(void *arg) {
  while (1) {
    pthread_mutex_lock(&mutex); 
    while (conn_fd_counter == 0) 
      pthread_cond_wait(&fill, &mutex);
    int conn_fd = get_conn_fd();
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);

    request_handle(conn_fd);
    close_or_die(conn_fd);
  }
}

//
// ./wserver [-d <basedir>] [-p <portnum>] [-t threads]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    int threads = 1;
    
    // parse command line options
    while ((c = getopt(argc, argv, "d:p:t:")) != -1) {
      switch (c) {
      case 'd':
          root_dir = optarg;
          break;
      case 'p':
          port = atoi(optarg);
          break;
      case 't':
          threads = atoi(optarg);
          break;
      default:
          fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
          exit(1);
	    }
    }

    // run the woker threads
    pthread_t worker_threads[threads];
    for (int i = 0; i < threads; i++) {
      if (pthread_create(&worker_threads[i], NULL, request_worker, NULL) != 0) {
        perror("pthread_create");
        exit(1);
      }
    }

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
      struct sockaddr_in client_addr;
      int client_len = sizeof(client_addr);
      int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
      pthread_mutex_lock(&mutex); 
      while (conn_fd_counter == MAXBUF) 
        pthread_cond_wait(&empty, &mutex);
      put_conn_fd(conn_fd);
      pthread_cond_signal(&fill);
      pthread_mutex_unlock(&mutex);
    }

    return 0;
}


    


 
