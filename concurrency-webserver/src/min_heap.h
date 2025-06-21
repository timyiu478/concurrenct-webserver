#ifndef __MIN_HEAP_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct HeapNode {
    int priority;
    void *value;
    struct HeapNode *left;
    struct HeapNode *right;
    struct HeapNode *parent;
} HeapNode;

typedef struct {
    HeapNode *root;
    int size;
} MinHeap;

typedef struct QueueNode {
    HeapNode *heapNode;
    struct QueueNode *next;
} QueueNode;

// Queue for bfs
typedef struct {
    QueueNode *front, *rear;
} Queue;



void insert(MinHeap *heap, int priority, void *value);
void* extractMin(MinHeap *heap);
void freeMinHeap(MinHeap *heap);

#endif // __MIN_HEAP_H__
