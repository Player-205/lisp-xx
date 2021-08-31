#include <iostream>
#include <csetjmp>
 
std::jmp_buf jump_buffer;


int main()
{
    volatile int count = 0; // modified locals in setjmp scope must be volatile
    if (setjmp(jump_buffer) != 9) { // equality against constant expression in an if
        ++count;  // This will cause setjmp() to exit
        std::cout << "a(" << count << ") called\n";
        std::longjmp(jump_buffer, count+1);  // setjmp() will return count+1
    }
}