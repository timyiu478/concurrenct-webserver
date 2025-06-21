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

HeapNode* createNode(int priority, void *value) {
    HeapNode *node = malloc(sizeof(HeapNode));
    node->priority = priority;
    node->value = value;
    node->left = node->right = node->parent = NULL;
    return node;
}

// Swap only the contents (not the pointers)
void swapNodes(HeapNode *a, HeapNode *b) {
    int tempPriority = a->priority;
    void *tempValue = a->value;
    a->priority = b->priority;
    a->value = b->value;
    b->priority = tempPriority;
    b->value = tempValue;
}

Queue* createQueue() {
    Queue *q = malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue *q, HeapNode *node) {
    QueueNode *newNode = malloc(sizeof(QueueNode));
    newNode->heapNode = node;
    newNode->next = NULL;
    if (!q->rear) q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

HeapNode* dequeue(Queue *q) {
    if (!q->front) return NULL;
    QueueNode *temp = q->front;
    HeapNode *node = temp->heapNode;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    free(temp);
    return node;
}

void freeQueue(Queue *q) {
    while (q->front) dequeue(q);
    free(q);
}

void heapifyUp(HeapNode *node) {
    while (node->parent && node->priority < node->parent->priority) {
        swapNodes(node, node->parent);
        node = node->parent;
    }
}

void heapifyDown(HeapNode *node) {
    while (node) {
        HeapNode *smallest = node;
        if (node->left && node->left->priority < smallest->priority)
            smallest = node->left;
        if (node->right && node->right->priority < smallest->priority)
            smallest = node->right;

        if (smallest == node) break;

        swapNodes(node, smallest);
        node = smallest;
    }
}

void insert(MinHeap *heap, int priority, void *value) {
    HeapNode *newNode = createNode(priority, value);
    heap->size++;

    if (!heap->root) {
        heap->root = newNode;
        return;
    }

    Queue *q = createQueue();
    enqueue(q, heap->root);

    while (q->front) {
        HeapNode *current = dequeue(q);

        if (!current->left) {
            current->left = newNode;
            newNode->parent = current;
            break;
        } else enqueue(q, current->left);

        if (!current->right) {
            current->right = newNode;
            newNode->parent = current;
            break;
        } else enqueue(q, current->right);
    }

    freeQueue(q);
    heapifyUp(newNode);
}

void* extractMin(MinHeap *heap) {
    if (!heap || !heap->root) return NULL;

    void *minValue = heap->root->value;

    if (heap->size == 1) {
        free(heap->root);
        heap->root = NULL;
        heap->size = 0;
        return minValue;
    }

    // Find last node
    Queue *q = createQueue();
    enqueue(q, heap->root);
    HeapNode *last = NULL;

    while (q->front) {
        last = dequeue(q);
        if (last->left) enqueue(q, last->left);
        if (last->right) enqueue(q, last->right);
    }
    freeQueue(q);

    // Replace root with last node's data
    heap->root->priority = last->priority;
    heap->root->value = last->value;

    // Remove last node from tree
    HeapNode *parent = last->parent;
    if (parent->left == last) parent->left = NULL;
    else if (parent->right == last) parent->right = NULL;
    free(last);

    heap->size--;
    heapifyDown(heap->root);

    return minValue;
}

void freeHeapNodes(HeapNode *node) {
    if (node == NULL)
        return;

    // Recursively free children
    freeHeapNodes(node->left);
    freeHeapNodes(node->right);

    // Free associated value if dynamically allocated
    // Example: free(node->value);

    free(node);  // Free the node itself
}

void freeMinHeap(MinHeap *heap) {
    freeHeapNodes(heap->root);
    heap->root = NULL;
    heap->size = 0;
}
