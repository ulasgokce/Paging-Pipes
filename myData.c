#include <sys/wait.h> // wait() Fonksiyonu Kütüphanesi
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// getch() Fonksiyonu İçin MacOS veya Linux Kütüphane Seçimi
#ifdef __APPLE__
#include <termios.h>
#else
#include <termio.h>
#endif

#define BUFFER_SIZE 500         // Satır çok uzun olduğunda konsol yada terminal satırı ikiye bölüyor. Eğer bu limitlemeyi kullanmazsak, satır sayısı artar ve bunu kontrol edemeyiz.
#define MAXIMUM_LINE_NUMBER 100 // Dosyadaki maksimum satır sayısı
#define LINES_PER_TURN 24       // myMore ile basılacak satır sayısı
#define READ_END 0
#define WRITE_END 1

// Fonksiyonun amacı kullanıcının devam etmek için girdiği tuşu kontrol eder. MacOS'da termios.h, Linux'da termio.h kütüphaneleri bu fonksiyon için kullanılır.
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

    return ch;
}

// Fonksiyonunun amacı byte bazında dosyanın boyutunu verir.
int fileSize(FILE *fp)
{
    fseek(fp, 0L, SEEK_END);
    int fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    return fileSize;
}

// Fonksiyonun amacı dosyanın işaretçisini verir.
FILE *getFile(char *filePath)
{
    FILE *fp;
    fp = fopen(filePath, "r");

    if (fp != NULL)
        return fp;

    else
    {
        perror("Error");
        exit(1);
    }
}

// Fonksiyonun amacı dosyayı okurken aynı anda ekran bastırır.
int readAllFile(FILE *fp)
{
    int byteOfFile = fileSize(fp);
    char *records = malloc(byteOfFile + 1);

    if (!fp)
        return -1;

    while (fgets(records, byteOfFile, fp) != NULL) // Dosyanın sonuna(\0) kadar okur.
        printf("%s", records);

    fclose(fp);

    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fp = getFile(argv[1]);

    // Eğer girdi "./myData <filename>" ise bütün dosyayı okur.
    if (argc == 2)
    {
        readAllFile(fp);
    }
    // Eğer girdi "./myData <filename> = myMore" ise dosyayı 24 satırlık parçalar halinde okur.
    else if (!strcmp(argv[2], "=") && !strcmp(argv[3], "myMore") && argc == 4)
    {
        char buffer[BUFFER_SIZE];
        char inputChar;                                   // Bu değişken girdileri tutar.
        int isFinished = 0;                               // Bu değişken bitip bitmediğini tutar.
        char write_msg[MAXIMUM_LINE_NUMBER][BUFFER_SIZE]; // Bu değişken pipe aracılığıyla gönderilen değişkeni tutar.

        do
        {
            pid_t pid;
            int fd[2];
            pipe(fd);
            pid = fork(); // Program çatallandırıldı.

            // Çatallandırma başarısız olursa buraya girecek.
            if (pid < 0)
            {
                fprintf(stderr, "Çatallandırma başarısız oldu.");
                return 1;
            }

            // Child Process
            if (pid == 0)
            {
                close(fd[WRITE_END]);

                char read_end_pipe[10]; // Bu değişken pipe'ın okuma ucunu tutar.
                sprintf(read_end_pipe, "%d", fd[READ_END]);

                char *arguments[3] = {argv[3], read_end_pipe, NULL}; // Bu değişken myMore programına gönderilecek olan parametrelerin dizisini tutar.

                // Execv fonksiyonu ile myMore'u çalıştırırız.
                if (execv(argv[3], arguments) == -1)
                {
                    perror("myMore programı çalıştırılamadı.");
                    return -1;
                }
            }

            // Parent Process
            if (pid > 0)
            {
                int lineCount = 0; // Bu değişken kaç satır okunduğunu tutar.

                //24 satır okur eğer 24 satırdan daha az kaldıysa tamamını okur.
                for (int i = 0; i < LINES_PER_TURN; ++i)
                {
                    // Satırları okur
                    if (fgets(buffer, sizeof(buffer), fp) > 0)
                    {
                        strcpy(write_msg[i], buffer);
                        lineCount++;
                    }
                    // Dosya okuma bittiyse döngüden çıkar
                    else
                    {
                        isFinished = 1;
                        break;
                    }
                }

                close(fd[READ_END]);
                write(fd[WRITE_END], &write_msg, lineCount * BUFFER_SIZE);
                close(fd[WRITE_END]);

                wait(NULL);
            }

            if (isFinished == 0)
                printf("\n ********************* DEVAM ETMEK İÇİN SPACE TUŞUNA BASIN *********************\n\n");
            else
            {
                printf("\n ********************* DOSYA TAMAMEN OKUNDU *********************\n\n");
                return 0;
            }

            inputChar = getch(); //Kullanıcının girdisi

        } while (isFinished != 1 && inputChar != 'q');
    }
    else
    {
        printf("\nKullanım Hatası!\nDosyanın tamamını okumak için:\t\t\"./myData <Metin Dosyası Yolu>\"\nDosyayı satır satır okumak için:\t\"./myData <Metin Dosyası Yolu> = myMore\"\n");
        return 1;
    }

    return 0;
}
