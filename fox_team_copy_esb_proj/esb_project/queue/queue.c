#include "./queue.h"

// A linked list (LL) node to store a queue entry
struct QNode {
	char key[45];
	char status[20];
	int number_of_attempts;
	struct QNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
typedef struct Queue {
	struct QNode *front, *rear;
} Queue;
Queue *q;

// A utility function to create a new linked list node.
struct QNode* newNode(char *k)
{
	struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
	strcpy(temp->key, k);
	strcpy(temp->status, "available");
	temp->number_of_attempts = 0;
	temp->next = NULL;
	return temp;
}

// A utility function to create an empty queue
struct Queue* createQueue()
{
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->front = q->rear = NULL;
	return q;
}

// The function to add a key k to q
void enQueue(struct Queue* q, char *k)
{
	// printf("\nthis is it:%s\n\n",k);
	// Create a new LL node
	struct QNode* temp = newNode(k);

	// If queue is empty, then new node is front and rear both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
}

// Function to remove a key from given queue q
void deQueue(struct Queue* q)
{
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return;

	// Store previous front and move front one node ahead
	struct QNode* temp = q->front;

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	free(temp);
}
