// Bank-account program using random-access file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    char password[10];
    char type[10];
    double balance;
};

// Function prototypes
int generateOTP(void);
int login(FILE *fPtr, unsigned int *acc);
void deposit(FILE *fPtr, unsigned int acc);
void withdraw(FILE *fPtr, unsigned int acc);
void atmMenu(FILE *fPtr, unsigned int acc);
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

int generateOTP(void)
{
    return rand() % 9000 + 1000;
}

int login(FILE *fPtr, unsigned int *acc)
{
    struct clientData client;
    char pass[10];

    printf("Enter Account Number: ");
    scanf("%u", acc);

    if (*acc < 1 || *acc > 100)
    {
        printf("Invalid account number!\n");
        return 0;
    }

    printf("Enter Password: ");
    scanf("%9s", pass);

    fseek(fPtr, (*acc - 1) * sizeof(struct clientData), SEEK_SET);

    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading file!\n");
        return 0;
    }

    if (client.acctNum == 0 || strcmp(client.password, pass) != 0)
    {
        printf("Invalid login!\n");
        return 0;
    }

    int otp = generateOTP();
    int userOtp;

    printf("OTP: %d\n", otp);
    printf("Enter OTP: ");
    scanf("%d", &userOtp);

    if (otp == userOtp)
    {
        return 1;
    }

    printf("Wrong OTP!\n");
    return 0;
}

void deposit(FILE *fPtr, unsigned int acc)
{
    struct clientData client;
    double amt;

    printf("Enter amount to deposit: ");
    scanf("%lf", &amt);

    if (amt <= 0)
    {
        printf("Invalid amount!\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    client.balance += amt;

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    FILE *log = fopen("transactions.txt", "a");

    if (log != NULL)
    {
        fprintf(log, "Acc %u Deposited %.2f\n", acc, amt);
        fclose(log);
    }

    printf("Deposit successful!\n");
    printf("Current Balance: %.2f\n", client.balance);
}

void withdraw(FILE *fPtr, unsigned int acc)
{
    struct clientData client;
    double amt;

    printf("Enter amount to withdraw: ");
    scanf("%lf", &amt);

    if (amt <= 0)
    {
        printf("Invalid amount!\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (strcmp(client.type, "Savings") == 0 &&
        client.balance - amt < 500)
    {
        printf("Minimum balance of 500 required!\n");
        return;
    }

    if (amt > client.balance)
    {
        printf("Insufficient balance!\n");
        return;
    }

    client.balance -= amt;

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    FILE *log = fopen("transactions.txt", "a");

    if (log != NULL)
    {
        fprintf(log, "Acc %u Withdraw %.2f\n", acc, amt);
        fclose(log);
    }

    printf("Withdraw successful!\n");
    printf("Current Balance: %.2f\n", client.balance);
}

void atmMenu(FILE *fPtr, unsigned int acc)
{
    int ch;

    while (1)
    {
        printf("\n--- ATM MENU ---\n");
        printf("1. Deposit\n");
        printf("2. Withdraw\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        scanf("%d", &ch);

        switch (ch)
        {
        case 1:
            deposit(fPtr, acc);
            break;

        case 2:
            withdraw(fPtr, acc);
            break;

        case 3:
            return;

        default:
            printf("Invalid choice!\n");
        }
    }
}

int main()
{
    FILE *cfPtr;
    unsigned int choice;

    srand(time(NULL));

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("File could not be opened.\n");
        exit(EXIT_FAILURE);
    }

    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;

        case 2:
            updateRecord(cfPtr);
            break;

        case 3:
            newRecord(cfPtr);
            break;

        case 4:
            deleteRecord(cfPtr);
            break;

        case 5:
        {
            unsigned int acc;

            if (login(cfPtr, &acc))
            {
                printf("Login successful!\n");
                atmMenu(cfPtr, acc);
            }

            break;
        }

        default:
            printf("Incorrect choice!\n");
        }
    }

    fclose(cfPtr);

    return 0;
}

// create formatted text file
void textFile(FILE *readPtr)
{
    FILE *writePtr;

    struct clientData client = {0, "", "", "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        printf("File could not be opened.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr,
            "%-6s%-16s%-11s%-10s%10s\n",
            "Acct",
            "Last Name",
            "First Name",
            "Type",
            "Balance");

    while (fread(&client,
                 sizeof(struct clientData),
                 1,
                 readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr,
                    "%-6u%-16s%-11s%-10s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.type,
                    client.balance);
        }
    }

    fclose(writePtr);

    printf("accounts.txt created successfully!\n");
}

// update balance
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;

    struct clientData client = {0, "", "", "", "", 0.0};

    printf("Enter account to update (1 - 100): ");
    scanf("%u", &account);

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum == 0)
    {
        printf("Account not found!\n");
        return;
    }

    printf("Current balance: %.2f\n", client.balance);

    printf("Enter amount (+deposit / -withdraw): ");
    scanf("%lf", &transaction);

    client.balance += transaction;

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    fflush(fPtr);

    printf("Updated balance: %.2f\n", client.balance);
}

// delete record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", "", "", 0.0};

    unsigned int accountNum;

    printf("Enter account number to delete: ");
    scanf("%u", &accountNum);

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&blankClient,
           sizeof(struct clientData),
           1,
           fPtr);

    fflush(fPtr);

    printf("Account deleted successfully!\n");
}

// add new record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", "", "", 0.0};

    unsigned int accountNum;

    printf("Enter new account number (1 - 100): ");
    scanf("%u", &accountNum);

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum != 0)
    {
        printf("Account already exists!\n");
        return;
    }

    client.acctNum = accountNum;

    printf("Enter lastname: ");
    scanf("%14s", client.lastName);

    printf("Enter firstname: ");
    scanf("%9s", client.firstName);

    printf("Enter password: ");
    scanf("%9s", client.password);

    printf("Enter type (Savings/Current): ");
    scanf("%9s", client.type);

    printf("Enter balance: ");
    scanf("%lf", &client.balance);

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    fflush(fPtr);

    printf("Account created successfully!\n");
}

// menu
unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n");
    printf("1 - Print accounts\n");
    printf("2 - Update account\n");
    printf("3 - Add account\n");
    printf("4 - Delete account\n");
    printf("5 - Login (ATM)\n");
    printf("6 - Exit\n");
    printf("? ");

    scanf("%u", &menuChoice);

    return menuChoice;
}