//
//  main.c
//  PH_ATM_Simulator
//
//  Created by Rong Liao on 2025-10-11.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BALANCE_FILE "balance.txt"
#define TRANSACTION_FILE "transactions.txt"

// Function prototypes
void displayMenu();
void viewBalance();
void deposit();
void withdraw();
void viewTransactions();
double getBalance();
void updateBalance(double newBalance);
void logTransaction(const char* type, double amount, double balanceAfter);
void clearInputBuffer();

int main(int argc, const char * argv[]) {
    int choice;
    
    // DISABLE Buffering on stderr to ensure logs appear immediately
    setbuf(stderr, NULL);
    
    fprintf(stderr, "[LOG] ATM Simulator starting...\n");
    
    printf("========================================\n");
    printf("   Welcome to PH ATM Simulator\n");
    printf("========================================\n\n");
    
    // Initialize balance file if it doesn't exist
    FILE *balanceFile = fopen(BALANCE_FILE, "r");
    if (balanceFile == NULL) {
        fprintf(stderr, "[LOG] Balance file not found, creating new file\n");
        balanceFile = fopen(BALANCE_FILE, "w");
        fprintf(balanceFile, "0.00");
        fclose(balanceFile);
        printf("Account initialized with balance: $0.00\n\n");
        fprintf(stderr, "[LOG] Balance file created with initial balance: 0.00\n");
    } else {
        fclose(balanceFile);
        fprintf(stderr, "[LOG] Balance file found and loaded\n");
    }
    
    fprintf(stderr, "[LOG] Entering main menu loop\n");
    
    // Main menu loop
    while (1) {
        displayMenu();
        printf("Enter your choice (default: 1): ");
        
        // Read input and check if it's empty (just Enter pressed)
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "[LOG] Error reading input from user\n");
            printf("\nError reading input.\n\n");
            continue;
        }
        
        // If user just pressed Enter, use default option 1
        if (input[0] == '\n') {
            choice = 1;
            fprintf(stderr, "[LOG] User pressed Enter, using default choice: 1\n");
        } else if (sscanf(input, "%d", &choice) != 1) {
            fprintf(stderr, "[LOG] Invalid input received: %s", input);
            printf("\nInvalid input! Please enter a number.\n\n");
            continue;
        } else {
            fprintf(stderr, "[LOG] User selected choice: %d\n", choice);
        }
        
        printf("\n");
        
        switch (choice) {
            case 1:
                fprintf(stderr, "[LOG] Executing: View Balance\n");
                viewBalance();
                break;
            case 2:
                fprintf(stderr, "[LOG] Executing: Deposit\n");
                deposit();
                break;
            case 3:
                fprintf(stderr, "[LOG] Executing: Withdraw\n");
                withdraw();
                break;
            case 4:
                fprintf(stderr, "[LOG] Executing: View Transactions\n");
                viewTransactions();
                break;
            case 5:
                fprintf(stderr, "[LOG] User selected Exit, shutting down\n");
                printf("Thank you for using PH ATM Simulator!\n");
                printf("Goodbye!\n");
                return 0;
            default:
                fprintf(stderr, "[LOG] Invalid choice: %d\n", choice);
                printf("Invalid choice! Please select 1-5.\n\n");
        }
    }
    
    return 0;
}

void displayMenu() {
    printf("========================================\n");
    printf("              MAIN MENU\n");
    printf("========================================\n");
    printf("1. View Balance\n");
    printf("2. Deposit\n");
    printf("3. Withdraw\n");
    printf("4. View Transactions\n");
    printf("5. Exit\n");
    printf("========================================\n");
}

void viewBalance() {
    double balance = getBalance();
    fprintf(stderr, "[LOG] View Balance - Current balance: %.2f\n", balance);
    printf("========================================\n");
    printf("         CURRENT BALANCE\n");
    printf("========================================\n");
    printf("Your current balance: $%.2f\n", balance);
    printf("========================================\n\n");
}

void deposit() {
    double amount;
    double currentBalance = getBalance();
    
    fprintf(stderr, "[LOG] Deposit - Current balance: %.2f\n", currentBalance);
    
    printf("========================================\n");
    printf("              DEPOSIT\n");
    printf("========================================\n");
    printf("Current balance: $%.2f\n", currentBalance);
    printf("Enter amount to deposit: $");
    
    if (scanf("%lf", &amount) != 1) {
        clearInputBuffer();
        fprintf(stderr, "[LOG] Deposit - Invalid input received\n");
        printf("Invalid input! Deposit cancelled.\n\n");
        return;
    }
    clearInputBuffer();
    
    if (amount <= 0) {
        fprintf(stderr, "[LOG] Deposit - Invalid amount: %.2f\n", amount);
        printf("Invalid amount! Amount must be positive.\n\n");
        return;
    }
    
    double newBalance = currentBalance + amount;
    updateBalance(newBalance);
    logTransaction("DEPOSIT", amount, newBalance);
    
    fprintf(stderr, "[LOG] Deposit - Amount: %.2f, New balance: %.2f\n", amount, newBalance);
    
    printf("\nDeposit successful!\n");
    printf("Amount deposited: $%.2f\n", amount);
    printf("New balance: $%.2f\n", newBalance);
    printf("========================================\n\n");
}

void withdraw() {
    double amount;
    double currentBalance = getBalance();
    
    fprintf(stderr, "[LOG] Withdraw - Current balance: %.2f\n", currentBalance);
    
    printf("========================================\n");
    printf("             WITHDRAWAL\n");
    printf("========================================\n");
    printf("Current balance: $%.2f\n", currentBalance);
    printf("Enter amount to withdraw: $");
    
    if (scanf("%lf", &amount) != 1) {
        clearInputBuffer();
        fprintf(stderr, "[LOG] Withdraw - Invalid input received\n");
        printf("Invalid input! Withdrawal cancelled.\n\n");
        return;
    }
    clearInputBuffer();
    
    if (amount <= 0) {
        fprintf(stderr, "[LOG] Withdraw - Invalid amount: %.2f\n", amount);
        printf("Invalid amount! Amount must be positive.\n\n");
        return;
    }
    
    if (amount > currentBalance) {
        fprintf(stderr, "[LOG] Withdraw - Insufficient funds. Requested: %.2f, Available: %.2f\n", amount, currentBalance);
        printf("Insufficient funds! Withdrawal cancelled.\n");
        printf("Your current balance is: $%.2f\n\n", currentBalance);
        return;
    }
    
    double newBalance = currentBalance - amount;
    updateBalance(newBalance);
    logTransaction("WITHDRAW", amount, newBalance);
    
    fprintf(stderr, "[LOG] Withdraw - Amount: %.2f, New balance: %.2f\n", amount, newBalance);
    
    printf("\nWithdrawal successful!\n");
    printf("Amount withdrawn: $%.2f\n", amount);
    printf("New balance: $%.2f\n", newBalance);
    printf("========================================\n\n");
}

void viewTransactions() {
    fprintf(stderr, "[LOG] View Transactions - Reading transaction history\n");
    
    printf("========================================\n");
    printf("        TRANSACTION HISTORY\n");
    printf("========================================\n");
    
    FILE *transFile = fopen(TRANSACTION_FILE, "r");
    if (transFile == NULL) {
        fprintf(stderr, "[LOG] View Transactions - No transaction file found\n");
        printf("No transactions found.\n");
        printf("========================================\n\n");
        return;
    }
    
    char line[256];
    int count = 0;
    
    while (fgets(line, sizeof(line), transFile)) {
        printf("%s", line);
        count++;
    }
    
    fclose(transFile);
    
    fprintf(stderr, "[LOG] View Transactions - Found %d transactions\n", count);
    
    if (count == 0) {
        printf("No transactions found.\n");
    }
    
    printf("========================================\n\n");
}

double getBalance() {
    FILE *balanceFile = fopen(BALANCE_FILE, "r");
    if (balanceFile == NULL) {
        fprintf(stderr, "[LOG] getBalance - Balance file not found, returning 0.00\n");
        return 0.0;
    }
    
    double balance;
    fscanf(balanceFile, "%lf", &balance);
    fclose(balanceFile);
    
    fprintf(stderr, "[LOG] getBalance - Read balance: %.2f\n", balance);
    
    return balance;
}

void updateBalance(double newBalance) {
    fprintf(stderr, "[LOG] updateBalance - Writing new balance: %.2f\n", newBalance);
    
    FILE *balanceFile = fopen(BALANCE_FILE, "w");
    if (balanceFile == NULL) {
        fprintf(stderr, "[LOG] updateBalance - Error opening balance file for writing\n");
        printf("Error updating balance!\n");
        return;
    }
    
    fprintf(balanceFile, "%.2f", newBalance);
    fclose(balanceFile);
    
    fprintf(stderr, "[LOG] updateBalance - Balance file updated successfully\n");
}

void logTransaction(const char* type, double amount, double balanceAfter) {
    fprintf(stderr, "[LOG] logTransaction - Type: %s, Amount: %.2f, Balance after: %.2f\n", 
            type, amount, balanceAfter);
    
    FILE *transFile = fopen(TRANSACTION_FILE, "a");
    if (transFile == NULL) {
        fprintf(stderr, "[LOG] logTransaction - Error opening transaction file\n");
        printf("Error logging transaction!\n");
        return;
    }
    
    // Get current date and time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    fprintf(transFile, "%04d-%02d-%02d %02d:%02d:%02d | %-10s | $%10.2f | Balance: $%.2f\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            type, amount, balanceAfter);
    
    fclose(transFile);
    
    fprintf(stderr, "[LOG] logTransaction - Transaction logged successfully\n");
}

/* Clear the input buffer to avoid leftover characters */
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
