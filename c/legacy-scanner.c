#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/evp.h>

// File Information Structure
typedef struct {
    char name[256];
    char path[1024];
    size_t size;
    char hash[65]; // SHA-256 hash (64 chars + null terminator)
} FileInfo;

// Function Prototypes
void scanDirectory(const char *directory, FileInfo **files, int *fileCount, int calculateHash);
void findDuplicatesByName(FileInfo *files, int fileCount);
void findDuplicatesByContent(FileInfo *files, int fileCount);
void calculateSHA256(const char *filePath, char *outputHash);

// Main Function
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./legacy_scanner -p path [-n | -h]\n");
        printf("Options:\n");
        printf("  -p path   Specify the path to scan\n");
        printf("  -n        Find duplicates by name\n");
        printf("  -h        Find duplicates by content\n");
        return 1;
    }

    char *path = NULL;
    int calculateHash = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            path = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0) {
            calculateHash = 0;
        } else if (strcmp(argv[i], "-h") == 0) {
            calculateHash = 1;
        }
    }

    if (path == NULL) {
        printf("Error: Path not provided.\n");
        return 1;
    }

    FileInfo *files = malloc(100000 * sizeof(FileInfo)); // Dynamically allocate memory for files
    if (!files) {
        perror("Failed to allocate memory");
        return 1;
    }

    int fileCount = 0;

    printf("Scanning directory: %s\n", path);
    scanDirectory(path, &files, &fileCount, calculateHash);

    printf("Scan complete. Found %d files.\n", fileCount);

    if (calculateHash) {
        findDuplicatesByContent(files, fileCount);
    } else {
        findDuplicatesByName(files, fileCount);
    }

    free(files); // Free allocated memory
    return 0;
}

// Iterative Directory Scanning
void scanDirectory(const char *directory, FileInfo **files, int *fileCount, int calculateHash) {
    struct dirent *entry;
    DIR *dir = opendir(directory);
    if (!dir) {
        printf("Error: Could not open directory %s\n", directory);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", directory, entry->d_name);

        struct stat fileStat;
        if (stat(filePath, &fileStat) == 0) {
            if (S_ISDIR(fileStat.st_mode)) {
                scanDirectory(filePath, files, fileCount, calculateHash); // Recurse into subdirectories
            } else if (S_ISREG(fileStat.st_mode)) {
                // Store file information
                strcpy((*files)[*fileCount].name, entry->d_name);
                strcpy((*files)[*fileCount].path, filePath);
                (*files)[*fileCount].size = fileStat.st_size;

                if (calculateHash) {
                    calculateSHA256(filePath, (*files)[*fileCount].hash);
                } else {
                    (*files)[*fileCount].hash[0] = '\0'; // No hash for name-based duplicates
                }

                (*fileCount)++;
                if (*fileCount % 100000 == 0) { // Resize if needed
                    *files = realloc(*files, (*fileCount + 100000) * sizeof(FileInfo));
                    if (!(*files)) {
                        perror("Failed to reallocate memory");
                        closedir(dir);
                        return;
                    }
                }
            }
        }
    }

    closedir(dir);
}

// Calculate SHA-256 Hash
void calculateSHA256(const char *filePath, char *outputHash) {
    FILE *file = fopen(filePath, "rb");
    if (!file) {
        strcpy(outputHash, "ERROR");
        return;
    }

    unsigned char buffer[8192];
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        EVP_DigestUpdate(ctx, buffer, bytesRead);
    }

    EVP_DigestFinal_ex(ctx, hash, &hashLength);
    EVP_MD_CTX_free(ctx);

    fclose(file);

    for (unsigned int i = 0; i < hashLength; ++i) {
        sprintf(outputHash + (i * 2), "%02x", hash[i]);
    }
    outputHash[64] = '\0';
}

// Find Duplicates by Name
void findDuplicatesByName(FileInfo *files, int fileCount) {
    printf("\nFinding duplicates by name...\n");
    for (int i = 0; i < fileCount; ++i) {
        for (int j = i + 1; j < fileCount; ++j) {
            if (strcmp(files[i].name, files[j].name) == 0) {
                printf("Duplicate: %s\n", files[i].name);
                printf("  Path: %s\n", files[i].path);
                printf("  Path: %s\n", files[j].path);
            }
        }
    }
}

// Find Duplicates by Content
void findDuplicatesByContent(FileInfo *files, int fileCount) {
    printf("\nFinding duplicates by content...\n");
    for (int i = 0; i < fileCount; ++i) {
        for (int j = i + 1; j < fileCount; ++j) {
            if (strcmp(files[i].hash, files[j].hash) == 0) {
                printf("Duplicate (Hash): %s\n", files[i].hash);
                printf("  Name: %s, Path: %s\n", files[i].name, files[i].path);
                printf("  Name: %s, Path: %s\n", files[j].name, files[j].path);
            }
        }
    }
}

