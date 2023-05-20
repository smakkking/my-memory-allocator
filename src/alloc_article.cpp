#include "alloc.h"
#include <unistd.h>
#include <pthread.h>
#include <string.h> 

union header_t
{   
    struct
    {
        size_t size;
        unsigned is_free;
        header_t *next;
    } s;
    char stub[16]; // align
};

header_t *head, *tail;
pthread_mutex_t global_malloc_lock;

void* my_malloc(size_t size)
{
    size_t total_size;
    void *block;

    header_t *header;
    if (!size)
        return NULL;

    
    pthread_mutex_lock(&global_malloc_lock);
    // сначала ищем в собственной структуре блоков
    header = get_free_block(size);

    if (header)
    {
        header->s.is_free = 0;
        pthread_mutex_lock(&global_malloc_lock);
        return (void*) (header + 1); 
        // почему именно по адресу header+1 лежит искомый блок?
        // адресная арифметика - потому что конечный адрес будет A(header) + 16, как раз то, что нужно

    }
    // если там нет, то запрашиваем у ОС
    total_size = sizeof(header) + size;
    block = sbrk(total_size);

    if (block == (void*) -1)
    {
        return NULL;
        pthread_mutex_lock(&global_malloc_lock);
    }

    // после запроса, добавляем блок с наш учетный список
    header = (header_t *) block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head)
        head = header;

    if (tail)
        tail->s.next = header;

    tail = header;
    pthread_mutex_lock(&global_malloc_lock);
    return (void*) (header + 1);
}

header_t* get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr)
    {
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}

void my_free(void *block)
{
    header_t *header, *tmp;
    void *programmbreak;

    if (!block)
        return;

    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t *) block - 1; // чтобы попасть именно в адрес заголовка

    programmbreak = sbrk(0);
    // если freeing блок - последний, то вернем его ОС 
    if ((char *) block + header->s.size == programmbreak)
    {
        if (head == tail)
            head = tail = NULL;
        else
        {
            tmp = head;
            while (tmp)
            {
                if (tmp->s.next = tail)
                {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 -sizeof(header_t) - header->s.size);
    } 
    else 
    {
        // иначе - просто пометим как свободный
        header->s.is_free = 1;
    }
    
    pthread_mutex_lock(&global_malloc_lock);
}

void* my_calloc(size_t num, size_t nsize)
{
    size_t total_size;
    void *block;

    // почему не посчитать умножение?
    // хотим проверить на переполнение
    if (!num || !nsize)
        return NULL;

    total_size = num * nsize;
    if (total_size / num != nsize)
        return NULL;

    block = my_malloc(total_size);
    if (!block)
        return NULL;
    memset(block, 0, total_size);
    return block;

}

void* my_realloc(void* block, size_t size)
{
    header_t *header;
    void *ret;

    if (!block || !size)
        return my_malloc(size);

    // вообщее такая структура удобна, чтобы не искать поиском этот самый блок
    // просто делаешь преобразование и все - в моем собственном аллокаторе это не так.
    header = (header_t*) block - 1; 
    if (header->s.size >= size)
        return block;

    ret = my_malloc(size);
    if (ret)
    {
        memcpy(ret, block, header->s.size);
        my_free(block);
    }
    return ret;
}