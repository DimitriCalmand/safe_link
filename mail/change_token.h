#ifndef CHANGE_TOKEN_H
#define CHANGE_TOKEN_H
char* is_token_already_use();
char* refresh_token(char* refresh_token, char* client_id, char* client_secret);
char* get_mail();
void save_mail(char* mail);
#endif