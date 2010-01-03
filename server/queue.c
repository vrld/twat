/* This is a project for a seminar at the Hochschule fuer Gestaltung Karlsruhe.
 * Feel free to use it within the bounds of the following license:
 *
 * Copyright (c) 2010 Matthias Richter
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to ect to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise
 * to promote the sale, use or other dealings in this Software
 * without prior written authorization.
 */
#include <string.h>
#include "queue.h"
#include "../error.h"

Queue* queue_new()
{
    Queue* q;
    q = (Queue*)malloc(sizeof(Queue));
    CHECK_NULL(q, "cannot reserve space for queue");
    q->front = q->back = NULL;

    return q;
}

void queue_delete(Queue* q)
{
    Queue_item *temp;
    if (!q || !q->front) return;

    temp = q->front;
    do {
        dequeue(q);
        temp = temp->next;
    } while (temp);
    free(q);
}

void enqueue(Queue* q, const char* msg)
{
    Queue_item* temp;
    CHECK_NULL(q, "queue was null");

    temp = (Queue_item*)malloc(sizeof(Queue_item));
    CHECK_NULL(temp, "cannot reserve space for queue item");
    temp->message = (char*)malloc((strlen(msg) + 1) * sizeof(char));
    CHECK_NULL(temp->message, "cannot reserve space for message");
    strcpy(temp->message, msg);
    temp->next = NULL;

    if (q->back)
        q->back->next = temp;
    if (q->front == NULL)
        q->front = temp;
    q->back = temp;
}

void dequeue(Queue* q)
{
    Queue_item *temp;
    CHECK_NULL(q, "queue was null");

    temp = q->front;
    if (!temp) /* queue already empty! */
        return;

    q->front = temp->next;
    if (q->front == NULL)
        q->back = NULL;

    free(temp->message);
    free(temp);
    temp = NULL;
}

const char* front(Queue* q)
{
    CHECK_NULL(q, "queue was empty");
    if (queue_empty(q))
        error_and_exit("tried to access first element of empty queue!", __FILE__, "front(Queue *q)");

    return q->front->message;
}
