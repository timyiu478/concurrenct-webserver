#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "request.h"
#include "io_helper.h"
#include "pthread.h"
#include "min_heap.h"

#define SCHE_POLY_FIFO (0)
#define SCHE_POLY_SFF (1)

char default_root[] = ".";

int MAXREQBUF = 10;
int SCHE_POLY = SCHE_POLY_FIFO;

// queue for FIFO schedule policy
int *conn_fd_buffer;
int fill_ptr = 0;
int use_ptr = 0;
int conn_fd_counter = 0;

// min heap and task struct for SFF schedule policy
MinHeap min_heap;

typedef struct Task {
  int conn_fd;
  HTTPRequest *req;
} Task;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void put_conn_fd(int conn_fd, HTTPRequest *req) {
    if (SCHE_POLY == SCHE_POLY_FIFO) {
        pthread_mutex_lock(&mutex); 
        while (conn_fd_counter == MAXREQBUF) 
          pthread_cond_wait(&empty, &mutex);

        conn_fd_buffer[fill_ptr] = conn_fd;
        fill_ptr = (fill_ptr + 1) % MAXREQBUF;
        conn_fd_counter++;

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    } else if (SCHE_POLY == SCHE_POLY_SFF){
        pthread_mutex_lock(&mutex); 
        while (min_heap.size == MAXREQBUF) 
          pthread_cond_wait(&empty, &mutex);

        assert(req != NULL); 

        Task *task = (Task *) malloc(sizeof(Task));
        task->conn_fd = conn_fd;
        task->req = req;

        assert(task->req != NULL); 

        int fileSize = task->req->sbuf.st_size;

        insert(&min_heap, fileSize, task);

        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}

int get_conn_fd(HTTPRequest **req) {
    if (SCHE_POLY == SCHE_POLY_FIFO) {
        pthread_mutex_lock(&mutex); 
        while (conn_fd_counter == 0) 
          pthread_cond_wait(&fill, &mutex);

        if (use_ptr == fill_ptr) {
            return -1; // buffer is empty
        }

        int fd = conn_fd_buffer[use_ptr];
        use_ptr = (use_ptr + 1) % MAXREQBUF;
        conn_fd_counter--;

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        return fd;
    } else if (SCHE_POLY == SCHE_POLY_SFF){
        pthread_mutex_lock(&mutex); 
        while (min_heap.size == 0) 
          pthread_cond_wait(&fill, &mutex);
        
        Task *task = (Task *) extractMin(&min_heap);
        assert(task->req != NULL); 
        *req = task->req;
        int conn_fd = task->conn_fd;

        free(task);

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        return conn_fd;
    }
    // Invalid state
    return -1;
}

void *request_worker(void *arg) {
  if (SCHE_POLY == SCHE_POLY_FIFO) {
      while (1) {
        int conn_fd = get_conn_fd(NULL);
        request_handle(conn_fd);
        close_or_die(conn_fd);
      }
  } else if (SCHE_POLY == SCHE_POLY_SFF){
      while (1) {
        // sleep(10); // for testing purpose which allow buffering the requests in the min heap
        HTTPRequest *req = NULL;
        int conn_fd = get_conn_fd(&req);
        assert(conn_fd != 0); 
        assert(req != NULL); 
        printf("Serving uri:%s\n", req->uri);
        request_handle_without_parse(conn_fd, req);
        close_or_die(conn_fd);
        free(req);
      }
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
    while ((c = getopt(argc, argv, "d:p:t:s:b:")) != -1) {
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
      case 's':
          SCHE_POLY = atoi(optarg);
          break;
      case 'b':
          MAXREQBUF = atoi(optarg);
          break;
      default:
          fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-s schedule_policy] [-b buffers]\n");
          exit(1);
	    }
    }

    // Create connection buffer according to scheduling policy
    if (SCHE_POLY == SCHE_POLY_FIFO) {
      conn_fd_buffer = malloc(sizeof(int) * MAXREQBUF);
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

      if (SCHE_POLY == SCHE_POLY_FIFO) {
        put_conn_fd(conn_fd, NULL);
      } else if (SCHE_POLY == SCHE_POLY_SFF) {
        HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));
        assert(req != NULL);
        request_parse(conn_fd, req);
        put_conn_fd(conn_fd, req);
      }
    }

    // Free connection buffer according to scheduling policy
    if (SCHE_POLY == SCHE_POLY_FIFO) {
      free(conn_fd_buffer);
    } else if (SCHE_POLY == SCHE_POLY_SFF) {
      freeMinHeap(&min_heap);
    }

    return 0;
}

