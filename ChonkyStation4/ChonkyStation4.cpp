#include <Common.hpp>
#include <Loaders/ELF/ELFLoader.hpp>


int main() {
    printf("ChonkyStation4\n\n");
	
    ELFLoader loader;

    try { 
        loader.load("H:\\PS4\\BREW00083\\eboot.elf");
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return -1;
    }

    return 0;
}