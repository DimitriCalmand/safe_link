#ifndef CLIENT_H
#define CLIENT_H
char* get_spam(char* ip, char* port);
char** get_email_spamer(char* list, int* size);
void send_spam(char* ip, char* port, char* mail);
#endif
