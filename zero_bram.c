#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BRAM_BASE_ADDR 0x80000000
#define BRAM_SIZE 8192  // Just 8KB for testing

int main() {
    int fd;
    volatile unsigned int* bram;
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        return 1;
    }
    
    printf("Successfully opened /dev/mem\n");
    
    // Memory-map the BRAM region
    bram = (unsigned int*) mmap(NULL, BRAM_SIZE, PROT_READ | PROT_WRITE, 
                               MAP_SHARED, fd, BRAM_BASE_ADDR);
    if (bram == MAP_FAILED) {
        perror("Failed to mmap BRAM");
        close(fd);
        return 1;
    }
    
    printf("Successfully mapped memory region 0x%08X to 0x%08X\n", 
          BRAM_BASE_ADDR, BRAM_BASE_ADDR + BRAM_SIZE - 1);
    
    // Read first word before writing
    printf("Initial value at 0x%08X: 0x%08X\n", BRAM_BASE_ADDR, bram[0]);
    
    // Fill BRAM with zero using 32-bit aligned writes, with progress indicators
    printf("Starting to write zeros...\n");
    for (int i = 0; i < BRAM_SIZE/4; i++) {
        bram[i] = 0;
        if (i % 512 == 0) { // Report every 2KB
            printf("Wrote up to offset 0x%X\n", i * 4);
        }
    }
    
    printf("Successfully zeroed %d bytes of BRAM\n", BRAM_SIZE);
    
    // Clean up
    munmap((void*)bram, BRAM_SIZE);
    close(fd);
    return 0;
}