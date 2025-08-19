#include <stdlib.h>

void func3() {

}

void func2() {
    func3();
}

void func1() {
    func2();
}
int main(int argc, char *argv[])
{
    func1();
    func2();
    func3();
    return EXIT_SUCCESS;
}
