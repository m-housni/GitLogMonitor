#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <windows.h>
#include <dirent.h>
#include <sys/stat.h>

void gitPull() {
    // pull latest changes
    char buffer[1024];
    system("cd GitLogMonitor && git pull > gitpull_new.txt");
    FILE* fp0 = fopen("GitLogMonitor/gitpull_new.txt", "r");
    int stop = 0;
    while (fgets(buffer, sizeof(buffer), fp0) != NULL && !stop ) {
        if( strstr(buffer, "Already up to date") != NULL ) {
            //showNotification("Up to date !",buffer);
            //Beep(300, 600);
        } else {
            // showNotification("New Commit !","check latest commits to resolve any starting conflict.");
            // Beep(300, 600);
        }
        stop = 1;
    }
    fclose(fp0);
}

void showNotification(const char *title, const char *message) {
    NOTIFYICONDATA notifyIconData = { sizeof(NOTIFYICONDATA) };

    notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    notifyIconData.hWnd = GetConsoleWindow();
    notifyIconData.uID = 1;
    notifyIconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    notifyIconData.uCallbackMessage = WM_USER + 1;
    snprintf(notifyIconData.szTip, sizeof(notifyIconData.szTip), "%s", title);

    Shell_NotifyIcon(NIM_ADD, &notifyIconData);

    // Show the notification
    Shell_NotifyIcon(NIM_MODIFY, &notifyIconData);
    MessageBox(NULL, message, title, MB_ICONINFORMATION);

    // Remove the notification icon
    Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
}

void addToGitignore(const char *filename) {
    // Open the .gitignore file in append mode
    FILE *gitignore = fopen(".gitignore", "a");

    if (gitignore == NULL) {
        perror("Error opening .gitignore");
        exit(EXIT_FAILURE);
    }

    // Check if the file name is already in the .gitignore file
    char line[256];
    while (fgets(line, sizeof(line), gitignore) != NULL) {
        if (strstr(line, filename) != NULL) {
            // File name is already in .gitignore
            fclose(gitignore);
            return;
        }
    }

    // File name not found, so add it
    fprintf(gitignore, "%s\n", filename);

    // Close the file
    fclose(gitignore);
}

void gitLog(int hoursAgo) {
    char command[100];
    sprintf(command, "cd GitLogMonitor && git log --raw --since=\"%d hour\" > gitlog_new.txt", hoursAgo);
    system(command);
}

void toolInfo() {
    printf("*****************************************************************************************************************************************\n");
    printf("*** GitLogMonitor / developed by m-housni / repo: https://github.com/m-housni/git-log-monitor / (c)MIT **********************************\n");
    printf("*****************************************************************************************************************************************\n\n");
    printf("*** Usage *** \n");
    printf("1. Put GitLogMonitor.exe on the root of the git repo you want to monitor \n");
    printf("2. Launch GitLogMonitor.exe and set your last commit date and the keywords or filenames you want to track \n");
    printf("3. You get a sound notification whenever a new commit relates to your tracked keywords (list of files given) \n");
    printf("4. If you need to update your last pull date or your keywords, please close and relaunch the tool \n");
    printf("5. If you would like to improve the tool, you can find the source code here: https://github.com/m-housni/git-log-monitor \n\n");
    printf("****************************************************************************************************************************************\n");
}

void lastPullInfo(int hoursAgo) {
    time_t now = time(0);
    printf("Analysis is based on the date of your last git pull as you informed:\n");
    printf("%d hours before you lanched the app at: %s\n\n",hoursAgo, ctime(&now));
}

void setConsoleWindowSize(int columns, int lines) {
    char command[100];
    snprintf(command, sizeof(command), "mode con: cols=%d lines=%d", columns, lines);
    system(command);
}

void maximizeConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_MAXIMIZE);
}

int main() {
    maximizeConsoleWindow();
    // initializations
    int hoursAgo, numberFiles, index;
    char str[50];
    char *repoUrl;
    char option[1];
    char files[100][50];
    char buffer[1024];
    int beep = 1;

    toolInfo();

    // input hoursAgo
    printf("When did you do your last git pull? (hours ago): ");
    scanf("%d", &hoursAgo);

    // add files to track
    printf("How many files you would like to track? ");
    scanf("%d", &numberFiles);
    for (int k = 0; k < numberFiles; k++) {
        printf("%d - Enter keyword or file name: ", k);
        scanf("%s", &str);
        strcpy(files[k], str);
    }

    // Create copy of repo
    sprintf(repoUrl, system("git config --get remote.origin.url"));
    system("robocopy . GitLogMonitor /E");

    system("cls");

    // add GitLogMonitor  to .gitignore
    addToGitignore("GitLogMonitor");
    addToGitignore("GitLogMonitor.exe");


    // infinite loop
    while ( TRUE ) {

        gitPull();
        system("cls");
        gitLog(hoursAgo);
        toolInfo();
        lastPullInfo(hoursAgo);


        // tracked files list
        printf("You are tracking the keywords / files bellow:\n");
        for (int k = 0; k < numberFiles; k++) {
            printf("%d : %s\n",k,files[k]);
        }

        // Read the output from the file
        FILE* fp = fopen("GitLogMonitor/gitlog_new.txt", "r");
        if (fp == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        printf("\n\n/!\\ These files need you attention,they were recently comitted and they contain your keywords! /!\\ \n\n");
        beep = 1;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            for ( int j = 0; j < numberFiles; j++ ) {
                if( strstr(buffer, files[j]) != NULL ) {
                    if( beep == 1 ) {
                        Beep(500, 3000);
                        //showNotification("Merge Conflict Risk At:",buffer);
                    }
                    printf(buffer);
                    usleep(200 * 1000);
                    beep = 0;
                }
            }
        }

        fclose(fp);

        sleep(60);
    }

}
