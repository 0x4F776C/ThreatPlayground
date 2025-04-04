#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char* selectTLD(int);
void generateDomain(char*, int, const char*);
int main();

const char* selectTLD(int hour) {
    if (hour >= 0 && hour < 6) {
        return ".com";
    } else if (hour >= 6 && hour < 12) {
        return ".net";
    } else if (hour >= 12 && hour < 18) {
        return ".org";
    } else {
        return ".io";
    }
}

void generateDomain(char* buffer, int length, const char* tld) {
    char charset[] = "abcdefghijklmnopqrstuvwxyz";
    int name_length = length - strlen(tld);

    for (int i = 0; i < name_length; i++) {
        buffer[i] = charset[rand() % 26];
    }

    strcpy(buffer + name_length, tld);
    buffer[length] = '\0';
}

int main() {
    srand(time(NULL));

    time_t now;
    time(&now);
    struct tm* local = localtime(&now);

    int hour = local->tm_hour;
    const char* tld = selectTLD(hour);

    int num_domains = 10;
    int domain_length = 12;

    for (int i = 0; i < num_domains; i++) {
        char domain[domain_length + 1];
        generateDomain(domain, domain_length, tld);

        //char full_url[20 + domain_length + 1];
        
        //sprintf(full_url, "tiny.cc/%s", domain);
        //printf("Generated domain: %s\n", full_url);
        printf("Generated domain: %s\n", domain);
    }

    return 0;
}
