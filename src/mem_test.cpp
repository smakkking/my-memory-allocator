#include "alloc.h"
#include <iostream>

void free_block_structure();
void print_mem();
void print_block_state();

int main() {
    using namespace std;

    cout << "---------------begin" << endl;
    print_block_state();

    char* t = (char*) my_malloc(2);
    cout << "---------------alloc 2 bytes" << endl;
    print_block_state();

    char* t2 = (char*) my_malloc(6);
    cout << "---------------alloc 6 bytes" << endl;
    print_block_state();

    cout << "---------------free 2 bytes" << endl;
    my_free(t);
    print_block_state();

    cout << "---------------free 6 bytes" << endl;
    my_free(t2);
    print_block_state();

    free_block_structure();
    return 0;
}