#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <glib.h>
#include "mail/access_token.h"
#include "mail/id_parser.h"
#include "mail/body_parser.h"
#include "mail/get_mail.h"
#include "mail/decodeb64.h"
#include "ia/model_tensorflow.h"
#include "ia/model.h"
#include "ia/black_list/client.h"
#include "mail/change_token.h"

GtkWidget *Menu;
GtkWidget *Mail;
GtkWidget *MailboxWindow;
GtkWidget *SpamboxWindow;
GtkWidget *MailboxView;
GtkWidget *SpamboxView;
GtkEntry *AuthEntry;
GtkEntry *EmailEntry;
GtkButton *AuthButton;
GtkButton *EmailButton;
GtkButton *ReturnButton;
GtkButton *MailButton;
GtkWidget *MailGridButton[1024];
GtkButton *SpamButton;
GtkWidget *SpamGridButton[1024];
GtkGrid *MailboxGrid;
GtkGrid *SpamboxGrid;
GtkLabel *AuthLabel;
GtkLabel *EmailLabel;

char *email;
char *dir_path;
char *ids_path;
char *mails_path;
char *spams_path;
DIR *mails_dir;
DIR *spams_dir;

int bool_is_connected = 0;
int mails = 0;
int spams = 0;

// Set the client ID and client secret.
char* client_id = "391645126883-hi7agjilffcrck31lcl740mljfk2defi.apps.googleusercontent.com";
char* client_secret = "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";

const char *auth_code;
const char *token;
char **ids;
char **contents;

char **mails_list;
char **spams_list;

model_spam *model;

void exit_program();
void on_Menu_destroy();
void on_Mail_destroy();
void on_MailButton_clicked();
void on_SpamButton_clicked();
void on_AuthButton_clicked();
void on_row_mail(GtkButton *button);
void on_row_spam(GtkButton *button);
void on_EmailButton_clicked();
void on_ReturnButton_clicked();
int check_path(char *path, char **dir_list, int dir_list_size);

char *ip_server;
char *port_server;
void lauch(char* ref_token)
{
    email = get_mail();
    bool_is_connected = 1;
    dir_path = NULL;
    int a = asprintf(&dir_path, "database/%s", email);
    if (a == -1)
    {
        errx(1, "asprintf() failed while creating the directory");
    }
    mails_path = NULL;
    int b = asprintf(&mails_path, "%s/mails", dir_path);
    if (b == -1)
    {
        errx(1, "asprintf() failed while creating the directory");
    }
    spams_path = NULL;
    int c = asprintf(&spams_path, "%s/spams", dir_path);
    if (c == -1)
    {
        errx(1, "asprintf() failed while creating the directory");
    }
    token = refresh_token(ref_token, client_id, client_secret);
    ids = NULL;

    ids = get_id(token);
    // Create a path to the "database/email.ids.txt" file.
    ids_path = NULL;
    a = asprintf(&ids_path, "%s/ids.txt", dir_path);
    if (a == -1)
    {
        errx(1, "asprintf() failed while creating the path");
    }
    add_ids(ids, ids_path, dir_path);
    // Get the contens of the mails.
    contents = NULL;
    contents = get_contents(ids, token, ids_path, dir_path);
    mails_dir = opendir(mails_path);
    spams_dir = opendir(spams_path);
    new_email(ids, dir_path, contents, ip_server, port_server, model, token);
    // Hide the Menu window.
    gtk_widget_hide(Menu);
    // Show the Mail window.
    gtk_widget_show_all(Mail);
    gtk_widget_hide(SpamboxWindow);
}
int main(int argc, char *argv[]) 
{
    if (argc != 3)
    {
        errx(1, "Usage: ./safelink <ip_server> <port_server>");
    }

    ip_server = argv[1];
    port_server = argv[2];

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Load the UI file
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "gui/interface.glade", NULL);

    // Assign the variables
    Menu = GTK_WIDGET(gtk_builder_get_object(builder, "Menu"));
    Mail = GTK_WIDGET(gtk_builder_get_object(builder, "Mail"));
    MailboxWindow = GTK_WIDGET(gtk_builder_get_object(builder, "MailboxWindow"));
    SpamboxWindow = GTK_WIDGET(gtk_builder_get_object(builder, "SpamboxWindow"));
    MailboxView = GTK_WIDGET(gtk_builder_get_object(builder, "MailboxView"));
    SpamboxView = GTK_WIDGET(gtk_builder_get_object(builder, "SpamboxView"));
    AuthEntry = GTK_ENTRY(gtk_builder_get_object(builder, "AuthEntry"));
    AuthButton = GTK_BUTTON(gtk_builder_get_object(builder, "AuthButton"));
    EmailEntry = GTK_ENTRY(gtk_builder_get_object(builder, "EmailEntry"));
    EmailButton = GTK_BUTTON(gtk_builder_get_object(builder, "EmailButton"));
    ReturnButton = GTK_BUTTON(gtk_builder_get_object(builder, "ReturnButton"));
    MailButton = GTK_BUTTON(gtk_builder_get_object(builder, "MailboxButton"));
    SpamButton = GTK_BUTTON(gtk_builder_get_object(builder, "SpamboxButton"));
    MailboxGrid = GTK_GRID(gtk_builder_get_object(builder, "MailboxGrid"));
    SpamboxGrid = GTK_GRID(gtk_builder_get_object(builder, "SpamboxGrid"));
    AuthLabel = GTK_LABEL(gtk_builder_get_object(builder, "AuthLabel"));
    EmailLabel = GTK_LABEL(gtk_builder_get_object(builder, "EmailLabel"));

    // Connect the signals
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    // Show the Menu window
    gtk_widget_show_all(Menu);

    // If the window is closed, exit the program
    g_signal_connect(Menu, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Malloc the memory for the mails and spams lists

    mails_list = malloc(1024 * sizeof(char*));
    spams_list = malloc(1024 * sizeof(char*));

    // Initialize the model
    model = init_model("./ia/model", "./ia/word_index.sv");

    // Start the main loop
    char* ref_token = is_token_already_use();
    if (ref_token[0] != '0')    
    {
        // Start the main loop
        lauch(ref_token);
        free(ref_token);

    }
    // free(ref_token);
    gtk_main();
    
    return 0;
}

// Exit the program
void exit_program()
{
    // Free the memory allocated for the mails_list and spams_list arrays.
    for (int i = 0; i < mails; i++)
    {
        free(mails_list[i]);
    }
    free(mails_list);
    for (int i = 0; i < spams; i++)
    {
        free(spams_list[i]);
    }
    free(spams_list);

    // Free all the memory allocated.
    free(ids_path);
    for (int i = 0; ids[i]!=NULL; i++)
    {
        free(ids[i]);
    }
    free(ids);
    free(dir_path);
    char *token = NULL;
    free(token);
    // free(port_server);
    model_free(model);
    for (int i = 0; contents[i] != NULL; i++)
    {
        free(contents[i]);
    }
    free(contents);
    // free(model);
    // for (int i = 0; i < mails; i++)
    // {
    //     free(MailGridButton[i]);
    // }
    // free(MailGridButton);
    // for (int i = 0; i < spams; i++)
    // {
    //     free(SpamGridButton[i]);
    // }
    // free(SpamGridButton);
    if (bool_is_connected)
    {
        free(email);
    }
    closedir(spams_dir);
    closedir(mails_dir);
    //destroy all the window
    
//     GtkWidget *Menu;
// GtkWidget *Mail;
// GtkWidget *MailboxWindow;
// GtkWidget *SpamboxWindow;
// GtkWidget *MailboxView;
// GtkWidget *SpamboxView;
// GtkEntry *AuthEntry;
// GtkEntry *EmailEntry;
// GtkButton *AuthButton;
// GtkButton *EmailButton;
// GtkButton *ReturnButton;
// GtkButton *MailButton;
// GtkWidget *MailGridButton[1024];
// GtkButton *SpamButton;
// GtkWidget *SpamGridButton[1024];
// GtkGrid *MailboxGrid;
// GtkGrid *SpamboxGrid;
// GtkLabel *AuthLabel;
// GtkLabel *EmailLabel;
    gtk_widget_destroy(Menu);
    gtk_widget_destroy(Mail);
    gtk_widget_destroy(MailboxWindow);
    gtk_widget_destroy(SpamboxWindow);

    gtk_main_quit();
}

// On EmailButton click
void on_EmailButton_clicked()
{
    // Get the entry text
    const gchar *entry_text;
    entry_text = gtk_entry_get_text(GTK_ENTRY(EmailEntry));
    // Check if the entry text is empty
    if (strcmp(entry_text, "") == 0)
    {
        // Set the label text
        gtk_label_set_text(EmailLabel, "Please enter an email address.");
    }
    else
    {
        gtk_label_set_text(EmailLabel, "Email address saved.");

        email = g_strdup(entry_text);
        // We want to create a directory with the name of the email address.
        dir_path = NULL;
        save_mail(email);
        int a = asprintf(&dir_path, "database/%s", email);
        if (a == -1)
        {
            errx(1, "asprintf() failed while creating the directory");
        }
        mkdir(dir_path, 0777);

        // Create the directories mails and spams within the directory created above.
        mails_path = NULL;
        int b = asprintf(&mails_path, "%s/mails", dir_path);
        if (b == -1)
        {
            errx(1, "asprintf() failed while creating the directory");
        }
        mkdir(mails_path, 0777);
        spams_path = NULL;
        int c = asprintf(&spams_path, "%s/spams", dir_path);
        if (c == -1)
        {
            errx(1, "asprintf() failed while creating the directory");
        }
        mkdir(spams_path, 0777);

        // Wait 3 seconds
        //sleep(3);
        // Launch the authorization_code function.
        mails_dir = opendir(mails_path);
        spams_dir = opendir(spams_path);
        authorization_code();
    }
}

// On MailButton click
void on_MailboxButton_clicked()
{
   
   
    // Show the MailboxWindow
    gtk_widget_show_all(MailboxWindow);
    gtk_widget_hide(SpamboxWindow);

    // We want to go to the mails directory and get the number of files in it to know how many mails there are.
    struct dirent *entry;

    // Open the directory
    
    if (mails_dir == NULL)
    {
        errx(1, "opendir() failed while opening the mails directory");
    }

    // Get the number of files in the directory
    while ((entry = readdir(mails_dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
        {
            continue;
        }

        // Update the path to the file
        char *mails_file_path = NULL;
        int a = asprintf(&mails_file_path, "%s/%s", mails_path, entry->d_name);
        if (a == -1)
        {
            errx(1, "asprintf() failed while updating the path to the file");
        }

        if (check_path(mails_file_path, mails_list, mails) == 1)
        {
            free(mails_file_path);
            continue;
        }
        // Add the mails_file_path to the mails_list array to be able to access it later.
        mails_list[mails] = malloc(strlen(mails_file_path) + 1);
        strcpy(mails_list[mails], mails_file_path);

        // Open the file
        FILE *mails_file = fopen(mails_file_path, "r");
        if (mails_file == NULL)
        {
            errx(1, "fopen() failed while opening the file");
        }
        // Read the first three lines of the file.
        char *mails_line1 = NULL;
        char *mails_line2 = NULL;
        char *mails_line3 = NULL;
        size_t mails_len1 = 0;
        size_t mails_len2 = 0;
        size_t mails_len3 = 0;
        ssize_t mails_read1 = getline(&mails_line1, &mails_len1, mails_file);
        ssize_t mails_read2 = getline(&mails_line2, &mails_len2, mails_file);
        ssize_t mails_read3 = getline(&mails_line3, &mails_len3, mails_file);
        if (mails_read1 == -1 || mails_read2 == -1 || mails_read3 == -1)
        {
            errx(1, "getline() failed while reading the first three lines of the file");
        }
        // Remove the newline character from the end of the first and second lines.
        mails_line1[strcspn(mails_line1, "\n")] = 0;
        mails_line2[strcspn(mails_line2, "\n")] = 0;

        // Cut the second line to only the date.
        char *mails_date = NULL;
        int c = asprintf(&mails_date, "%s", mails_line2);
        if (c == -1)
        {
            errx(1, "asprintf() failed while cutting the second line to only the date");
        }
        mails_date[17] = '\0';

        // Create a new string with the first line and the second line.
        char *mails_line = NULL;
        int b = asprintf(&mails_line, "%s: %s : %s", mails_line1, mails_date, mails_line3);
        if (b == -1)
        {
            errx(1, "asprintf() failed while creating a new string with the first line and the second line");
        }
        // Add a new row.
        gtk_grid_insert_row(MailboxGrid, mails);
        // Add buttons to the grid for each mail and spam found in the files mails.txt and spams.txt.
        MailGridButton[mails] = gtk_button_new_with_label(mails_line);
        gtk_widget_set_halign(MailGridButton[mails], GTK_ALIGN_FILL);
        // Set the button to expand horizontally
        gtk_widget_set_hexpand(MailGridButton[mails], TRUE);
        gtk_grid_attach(MailboxGrid, MailGridButton[mails], 0, mails, 1, 1);
        g_signal_connect(MailGridButton[mails], "clicked", G_CALLBACK(on_row_mail), NULL);

        // Close the file
        fclose(mails_file);

        // Update the mails variable
        mails++;

        // Free the memory
        free(mails_line1);
        free(mails_line2);
        free(mails_line3);
        free(mails_line);
        free(mails_date);
        free(mails_file_path);
    }
    // closedir(spams_dir);
}

// On SpamButton click
void on_SpamboxButton_clicked()
{
    // Show the SpamboxWindow

    gtk_widget_show_all(SpamboxWindow);
    gtk_widget_hide(MailboxWindow);

    // We want to go to the mails directory and get the number of files in it to know how many mails there are.
    struct dirent *entry;

    // Open the directory
    if (spams_dir == NULL)
    {
        errx(1, "opendir() failed while opening the spams directory");
    }

    // Get the number of files in the directory
    while ((entry = readdir(spams_dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
        {
            continue;
        }

        // Update the path to the file
        char *spams_file_path = NULL;
        int a = asprintf(&spams_file_path, "%s/%s", spams_path, entry->d_name);
        if (a == -1)
        {
            errx(1, "asprintf() failed while updating the path to the file");
        }

        if (check_path(spams_file_path, spams_list, spams) == 1)
        {
            free(spams_file_path);
            continue;
        }
        // Copy the path to the file to the spams_list array to be able to access it later.
        spams_list[spams] = malloc(strlen(spams_file_path) + 1);
        strcpy(spams_list[spams], spams_file_path);

        // Open the file
        FILE *spams_file = fopen(spams_file_path, "r");
        if (spams_file == NULL)
        {
            errx(1, "fopen() failed while opening the file");
        }
        // Read the first three lines of the file.
        char *spams_line1 = NULL;
        char *spams_line2 = NULL;
        char *spams_line3 = NULL;
        size_t spams_len1 = 0;
        size_t spams_len2 = 0;
        size_t spams_len3 = 0;
        ssize_t spams_read1 = getline(&spams_line1, &spams_len1, spams_file);
        ssize_t spams_read2 = getline(&spams_line2, &spams_len2, spams_file);
        ssize_t spams_read3 = getline(&spams_line3, &spams_len3, spams_file);
        if (spams_read1 == -1 || spams_read2 == -1 || spams_read3 == -1)
        {
            errx(1, "getline() failed while reading the first two lines of the file");
        }
        // Remove the newline character from the end of the first line.
        spams_line1[strcspn(spams_line1, "\n")] = 0;
        spams_line2[strcspn(spams_line2, "\n")] = 0;

        // Cut the second line to only the date.
        char *spams_date = NULL;
        int c = asprintf(&spams_date, "%s", spams_line2);
        if (c == -1)
        {
            errx(1, "asprintf() failed while cutting the second line to only the date");
        }
        spams_date[17] = '\0';

        // Create a new string with the first line and the second line.
        char *spams_line = NULL;
        int b = asprintf(&spams_line, "%s: %s: %s", spams_line1, spams_date, spams_line3);
        if (b == -1)
        {
            errx(1, "asprintf() failed while creating a new string with the first line and the second line");
        }
        // Add a new row.
        gtk_grid_insert_row(SpamboxGrid, spams);
        // Add buttons to the grid for each mail and spam found in the files mails.txt and spams.txt.
        SpamGridButton[spams] = gtk_button_new_with_label(spams_line);
        gtk_widget_set_halign(SpamGridButton[spams], GTK_ALIGN_FILL);
        // Set the button to expand horizontally
        gtk_widget_set_hexpand(SpamGridButton[spams], TRUE);
        gtk_grid_attach(SpamboxGrid, SpamGridButton[spams], 0, spams, 1, 1);
        g_signal_connect(SpamGridButton[spams], "clicked", G_CALLBACK(on_row_spam), NULL);

        // Close the file
        fclose(spams_file);

        // Update the mails variable
        spams++;

        // Free the memory
        free(spams_line1);
        free(spams_line2);
        free(spams_line3);
        free(spams_line);
        free(spams_date);
        free(spams_file_path);
    }
    //close the directory
    
    
}

// On AuthButton click
void on_AuthButton_clicked()
{
    // Get the entry text.
    const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(AuthEntry));

    // Check if the entry text starts with "4/".
    if (strncmp(entry_text, "4/", 2) != 0)
    {
        // Set the label text
        gtk_label_set_text(AuthLabel, "Please enter a valid authorization code.");
    }
    else
    {
        // Set the label text
        gtk_label_set_text(AuthLabel, "Authorization Code set successfully. You'll be redirected to the mailbox in a few seconds.");

        // Get the authorization code from the AuthEntry.
        auth_code = NULL;
        auth_code = gtk_entry_get_text(AuthEntry);
        // Get the access token.
        token = NULL;
        token = access_token(auth_code, client_id, client_secret);
        // Get the IDs.
        ids = NULL;
        ids = get_id(token);
        // Create a path to the "database/email.ids.txt" file.
        ids_path = NULL;
        int a = asprintf(&ids_path, "%s/ids.txt", dir_path);
        if (a == -1)
        {
            errx(1, "asprintf() failed while creating the path");
        }
        add_ids(ids, ids_path, dir_path);
        // Get the contens of the mails.
        contents = NULL;
        contents = get_contents(ids, token, ids_path, dir_path);
        new_email(ids, dir_path, contents, ip_server, port_server, model, token);
        // Hide the Menu window.
        gtk_widget_hide(Menu);
        // Show the Mail window.
        gtk_widget_show_all(Mail);
        gtk_widget_hide(SpamboxWindow);        
    }
}

// On row mail click
void on_row_mail(GtkButton *button)
{
    // Create a new window (pop-up) to show the mail or spam content.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), gtk_button_get_label(button));
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    
    // Get the position of the button in the grid.
    int position = 0;
    for (int i = 0; i < mails; i++)
    {
        if (gtk_button_get_label(GTK_BUTTON(MailGridButton[i])) == gtk_button_get_label(GTK_BUTTON(button)))    
        {
            position = i;
            break;
        }
    }

    // Open the file corresponding to the same position in the mails_list array.
    FILE *fp = fopen(mails_list[position], "r");
    if (fp == NULL)
    {
        errx(1, "fopen() failed while opening the file");
    }
    // Read the entire file line by line and store it in the tmp variable.
    fseek(fp, 0, SEEK_END);
    long fsize = 0;
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *tmp = NULL;
    tmp = malloc(fsize + 1);
    if (tmp == NULL)
    {
        errx(1, "malloc() failed while allocating memory for the tmp variable");
    }
    char ch;
    int i = 0;
    while ((ch = fgetc(fp)) != EOF)
    {
        tmp[i] = ch;
        i++;
    }
    tmp[i] = '\0';
    // Close the file.
    fclose(fp);

    // Create a new label with the tmp variable as text.
    GtkWidget *label = gtk_label_new(tmp);
    // Add the label to the window.
    gtk_container_add(GTK_CONTAINER(window), label);
    // Show the window.
    gtk_widget_show_all(window);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    free(tmp);
}

// On row spam click
void on_row_spam(GtkButton *button)
{
    // Create a new window (pop-up) to show the mail or spam content.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), gtk_button_get_label(button));
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    // Get the position of the button in the grid.
    int position = 0;
    for (int i = 0; i < spams; i++)
    {
        if (gtk_button_get_label(GTK_BUTTON(SpamGridButton[i])) == gtk_button_get_label(GTK_BUTTON(button)))
        {
            position = i;
            break;
        }
    }

    // Open the file corresponding to the same position in the spams_list array.
    FILE *fp = fopen(spams_list[position], "r");
    if (fp == NULL)
    {
        errx(1, "%s file not found!", spams_list[position]);
    }
    // Read the entire file line by line and store it in the tmp variable.
    fseek(fp, 0, SEEK_END);
    long fsize = 0;
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *tmp = NULL;
    tmp = malloc(fsize + 1);
    if (tmp == NULL)
    {
        errx(1, "malloc() failed while allocating memory for tmp");
    }
    char ch;
    int i = 0;
    while ((ch = fgetc(fp)) != EOF)
    {
        tmp[i] = ch;
        i++;
    }
    tmp[i] = '\0';
    // Close the file.
    fclose(fp);

    // Create a new label with the tmp variable as text.
    GtkWidget *label = gtk_label_new(tmp);
    // Add the label to the window.
    gtk_container_add(GTK_CONTAINER(window), label);
    // Show the window.
    gtk_widget_show_all(window);
    //create a signal to delete the window when the user close it
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

    free(tmp);
}

int check_path(char *path, char **dir_list, int dir_list_size)
{
    for (int i = 0; i < dir_list_size; i++)
    {
        if (strcmp(path, dir_list[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// On ReturnButton click
void on_ReturnButton_clicked()
{
    // Hide the Mail window.
    gtk_widget_hide(Mail);
    for (int i = 0; ids[i]!=NULL; i++)
    {
        free(ids[i]);
    }
    free(ids);
    for (int i = 0; contents[i] != NULL; i++)
    {
        free(contents[i]);
    }
    free(contents);
    for (int i = 0; i < mails; i++)
    {
        free(mails_list[i]);
    }
    free(mails_list);
    for (int i = 0; i < spams; i++)
    {
        free(spams_list[i]);
    }
    free(spams_list);
    if (bool_is_connected)
    {
        free(email);
    }
    free((char *)token);

    // Show the Menu window.
    gtk_widget_show_all(Menu);


}

void on_Mail_destroy()
{
    exit_program(NULL, NULL);
}
void on_Menus_destroy()
{
    exit_program(NULL, NULL);
}