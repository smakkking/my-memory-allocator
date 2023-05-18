#include <iostream>
#include <cstddef>

const int MEM_SIZE = 16;

char MEMORY_ARRAY[MEM_SIZE];
// главная область памяти, с которой мы будем работать

struct memblock
{
    void* ptr;
    size_t size;
    bool is_free;

    memblock *next;
    memblock *prev;
    memblock(void* pt = (void*) &MEMORY_ARRAY[0], size_t s = MEM_SIZE, memblock *n = nullptr, memblock *p = nullptr)
    {
        ptr = pt;
        size = s;
        next = n;
        prev = p;
        is_free = true;
    }
};

memblock *HEAD = new memblock;

void* my_malloc(size_t num)
{
    memblock* block = find_block(num);
    if (!block)
        return nullptr;

    block->is_free = false;

    if (block->size != num) 
    {
        // расщипляем блок
        memblock* new_block = new memblock(
            (void*) block->ptr + num,
            block->size - num
        );

        // вставка в двойной список (работет, только когда оба соседа не null)
        new_block->next = block->next;
        block->next = new_block;
        
        if (new_block->next)
            new_block->next->prev = new_block;
        new_block->prev = block;

        // изменяем размер исходного блока
        block->size = num;
    }

    return block->ptr;
}

void my_free(void *p)
{
    memblock *target;
    for (target = HEAD; target != nullptr; target = target->next)
        if (target->ptr == p)
    
    target->is_free = true;

    // смотрим влево
    memblock *it1 = target->prev;
    if (it1 && it1->is_free)
    {
        target->prev = it1->prev;
        if (it1->prev)
            it1->prev->next = target;

        target->ptr = it1->ptr;
        target->size += it1->size;

        delete it1;
    }

    memblock *it2 = target->next;
    if (it2 && it2->is_free)
    {   
        target->next = it2->next;
        if (it2->next)
            it2->next->prev = target;

        target->size += it2->size;

        delete it2;
    }

}

void* my_calloc(size_t num, size_t nsize)
{
    void* p = my_malloc(num * nsize);
    // no overflow check

    if (!p)
        return nullptr;
    
    char *x = (char*) p;
    for (int i = 0; i < num * nsize; i++)
        x[i] = 0;
    
    return p;
}

void* my_realloc(void *block, size_t size)
{
    memblock *target;
    for (target = HEAD; target != nullptr; target = target->next)
        if (target->ptr == block)
            break;
    
    void* p = malloc(size);
    if (p)
    {
        for (int i = 0; i < target->size; i++)
            ((char*) p)[i] = ((char*) target->ptr)[i];
    }
    return p;
}

memblock* find_block(size_t num)
{
    for (memblock *it = HEAD; it != nullptr; it = it->next)
        if (it->size >= num && it->is_free)
            return it;

    return nullptr;
}

// вызывается в конце
void free_block_structure()
{
    if (!HEAD->next) 
        delete HEAD;
    else
    {
        memblock *it = HEAD->next;
        while (it)
        {
            delete it->prev;
            it = it->next;
        }
    }  
}

void print_mem() 
{
    for (int i = 0; i < MEM_SIZE; i++)
        std::cout << MEMORY_ARRAY[i] << std::endl;
}

