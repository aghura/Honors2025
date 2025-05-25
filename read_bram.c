#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BRAM_BASE_ADDR 0x80000000
#define BRAM_SIZE (8 * 1024)  // Start with just 8KB for safety

int main() {
    int fd;
    volatile unsigned int* bram;
    FILE* output;
    
    // Open output file
    output = fopen("non_zero_bram.txt", "w");
    if (!output) {
        perror("Failed to open output file");
        return 1;
    }
    
    printf("Created output file non_zero_bram.txt\n");
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        fclose(output);
        return 1;
    }
    
    printf("Successfully opened /dev/mem\n");
    
    // Memory-map the BRAM region
    bram = (unsigned int*) mmap(NULL, BRAM_SIZE, PROT_READ, 
                               MAP_SHARED, fd, BRAM_BASE_ADDR);
    if (bram == MAP_FAILED) {
        perror("Failed to mmap BRAM");
        close(fd);
        fclose(output);
        return 1;
    }
    
    printf("Successfully mapped memory region 0x%08X - 0x%08X\n", 
          BRAM_BASE_ADDR, BRAM_BASE_ADDR + BRAM_SIZE - 1);
    
    // Scan for non-zero values
    fprintf(output, "Address\t\tValue\n");
    fprintf(output, "-------------------------\n");
    
    int non_zero_count = 0;
    
    for (int i = 0; i < BRAM_SIZE/4; i++) {
        if (bram[i] != 0) {
            fprintf(output, "0x%08X\t0x%08X\n", 
                   BRAM_BASE_ADDR + (i * 4), bram[i]);
            printf("Found non-zero value 0x%08X at address 0x%08X\n",
                  bram[i], BRAM_BASE_ADDR + (i * 4));
            non_zero_count++;
        }
        
        // Add progress indicator
        if (i % 512 == 0) {
            printf("Scanned up to offset 0x%X\n", i * 4);
        }
    }
    
    printf("Scan complete: found %d non-zero values\n", non_zero_count);
    fprintf(output, "\nTotal non-zero values: %d\n", non_zero_count);
    
    // Clean up
    munmap((void*)bram, BRAM_SIZE);
    close(fd);
    fclose(output);
    
    printf("Results written to non_zero_bram.txt\n");
    return 0;
}