#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

// -------------------------------
// ATM Simulator (single account)
// - Stores transactions in memory
// - Persists to CSV and JSON on exit
// - Auto-loads history from JSON (preferred) or CSV on start
// Amounts handled in cents to avoid FP errors
// -------------------------------

#define MAX_TX 200
#define CSV_PATH "transactions.csv"
#define JSON_PATH "transactions.json"

typedef enum {
    TX_DEPOSIT = 1,
    TX_WITHDRAW = 2
} TxType;

typedef struct {
    TxType type;
    long long amount_cents;     // positive amount for both deposit/withdraw
    long long balance_after;    // balance after this transaction
    time_t timestamp;
} Transaction;

typedef struct {
    long long balance_cents;    // current balance
    Transaction tx[MAX_TX];
    int tx_count;
} Account;

// -------------------------------
// Utility helpers
// -------------------------------

static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

// Parse integer from string safely; returns 1 if ok
static int parse_int(const char *s, int *out) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (s == end) return 0;            // no digits
    while (*end) {                      // ensure only trailing spaces
        if (!isspace((unsigned char)*end)) return 0;
        end++;
    }
    if (v < INT_MIN || v > INT_MAX) return 0;
    *out = (int)v;
    return 1;
}

// Converts a string like "123.45" or "123" into cents (e.g., 12345)
// Returns 1 on success, 0 on failure; negative values not allowed here
static int parse_amount_cents(const char *s, long long *out_cents) {
    if (!s || !*s) return 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '-') return 0;
    long long dollars = 0, cents = 0;
    const char *p = s;
    while (*p && *p != '.' && !isspace((unsigned char)*p)) {
        if (!isdigit((unsigned char)*p)) return 0;
        dollars = dollars * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        int digits = 0;
        while (*p && isdigit((unsigned char)*p) && digits < 2) {
            cents = cents * 10 + (*p - '0');
            p++; digits++;
        }
        if (digits == 1) cents *= 10; // 12.3 -> 12.30
        if (*p && isdigit((unsigned char)*p)) return 0; // >2 decimals
    }
    while (*p) { if (!isspace((unsigned char)*p)) return 0; p++; }
    *out_cents = dollars * 100 + cents;
    return 1;
}

static void format_amount(long long cents, char *out, size_t out_sz) {
    int negative = 0;
    if (cents < 0) { negative = 1; cents = -cents; }
    long long dollars = cents / 100;
    long long rem = cents % 100;
    snprintf(out, out_sz, "%s%lld.%02lld", negative ? "-" : "", dollars, rem);
}

static void print_timestamp(time_t t) {
    struct tm *lt = localtime(&t);
    char buf[64];
    if (lt && strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)) {
        printf("%s", buf);
    } else {
        printf("(time unavailable)");
    }
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

// -------------------------------
// CSV persistence
// -------------------------------

static int save_to_csv(const Account *acc, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    // header
    fprintf(f, "type,amount_cents,balance_after,timestamp\n");
    for (int i = 0; i < acc->tx_count; ++i) {
        const Transaction *tx = &acc->tx[i];
        fprintf(f, "%d,%lld,%lld,%lld\n", (int)tx->type,
                tx->amount_cents, tx->balance_after, (long long)tx->timestamp);
    }
    fclose(f);
    return 1;
}

static int load_from_csv(Account *acc, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    char line[256];
    // skip header if present
    if (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "type,", 5) != 0) {
            // first line is data, rewind to start
            fseek(f, 0, SEEK_SET);
        }
    }
    acc->balance_cents = 0;
    acc->tx_count = 0;
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        char *p = line;
        // simple CSV parse (no quoted fields)
        char *tok;
        int col = 0; int type = 0; long long amount = 0, bal = 0, ts = 0;
        while ((tok = strtok(col == 0 ? p : NULL, ",")) != NULL) {
            if (col == 0) type = atoi(tok);
            else if (col == 1) amount = atoll(tok);
            else if (col == 2) bal = atoll(tok);
            else if (col == 3) ts = atoll(tok);
            col++;
        }
        if (col >= 4 && acc->tx_count < MAX_TX) {
            acc->tx[acc->tx_count].type = (TxType)type;
            acc->tx[acc->tx_count].amount_cents = amount;
            acc->tx[acc->tx_count].balance_after = bal;
            acc->tx[acc->tx_count].timestamp = (time_t)ts;
            acc->tx_count++;
            acc->balance_cents = bal; // last row's balance is current
        }
    }
    fclose(f);
    return 1;
}

// -------------------------------
// Minimal JSON persistence (no external libs)
// Format:
// { "balance_cents": 123, "transactions": [ {"type":1,"amount":5000,"balance_after":5000,"timestamp":1700000000}, ... ] }
// -------------------------------

static int save_to_json(const Account *acc, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "{\n  \"balance_cents\": %lld,\n  \"transactions\": [\n", acc->balance_cents);
    for (int i = 0; i < acc->tx_count; ++i) {
        const Transaction *tx = &acc->tx[i];
        fprintf(f, "    {\"type\": %d, \"amount\": %lld, \"balance_after\": %lld, \"timestamp\": %lld}%s\n",
                (int)tx->type, tx->amount_cents, tx->balance_after, (long long)tx->timestamp,
                (i + 1 < acc->tx_count) ? "," : "");
    }
    fprintf(f, "  ]\n}\n");
    fclose(f);
    return 1;
}

static int load_from_json(Account *acc, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    // read whole file
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return 0; }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);

    acc->tx_count = 0;
    acc->balance_cents = 0;

    // balance_cents
    const char *bpos = strstr(buf, "\"balance_cents\"");
    if (bpos) {
        long long bal = 0;
        if (sscanf(bpos, "\"balance_cents\"%*[^0-9-]%lld", &bal) == 1) acc->balance_cents = bal;
    }

    // iterate transactions
    const char *p = buf;
    while ((p = strstr(p, "{\"type\"")) != NULL) {
        int type = 0; long long amount = 0, bal = 0, ts = 0;
        sscanf(p, "{\"type\"%*[^0-9-]%d%*[^0-9-]%lld%*[^0-9-]%lld%*[^0-9-]%lld",
               &type, &amount, &bal, &ts);
        if (acc->tx_count < MAX_TX) {
            acc->tx[acc->tx_count].type = (TxType)type;
            acc->tx[acc->tx_count].amount_cents = amount;
            acc->tx[acc->tx_count].balance_after = bal;
            acc->tx[acc->tx_count].timestamp = (time_t)ts;
            acc->tx_count++;
        }
        p += 8; // advance to avoid infinite loop
    }

    // If no tx parsed but balance present, keep balance; else set to last tx balance
    if (acc->tx_count > 0) acc->balance_cents = acc->tx[acc->tx_count - 1].balance_after;

    free(buf);
    return 1;
}

// -------------------------------
// Account operations
// -------------------------------

static void record_tx(Account *acc, TxType type, long long amount) {
    if (acc->tx_count >= MAX_TX) {
        // Simple policy: drop the oldest by shifting left
        memmove(&acc->tx[0], &acc->tx[1], (MAX_TX - 1) * sizeof(Transaction));
        acc->tx_count = MAX_TX - 1;
    }
    Transaction *tx = &acc->tx[acc->tx_count++];
    tx->type = type;
    tx->amount_cents = amount;
    tx->balance_after = acc->balance_cents;
    tx->timestamp = time(NULL);
}

static void deposit(Account *acc) {
    char buf[128];
    printf("Enter amount to deposit (e.g., 100 or 100.50): ");
    if (!read_line(buf, sizeof(buf))) return;
    long long cents = 0;
    if (!parse_amount_cents(buf, &cents) || cents <= 0) {
        printf("Invalid amount. Please try again.\n");
        return;
    }
    acc->balance_cents += cents;
    record_tx(acc, TX_DEPOSIT, cents);
    char amt[32]; format_amount(cents, amt, sizeof(amt));
    char bal[32]; format_amount(acc->balance_cents, bal, sizeof(bal));
    printf("Deposited %s. New balance: %s\n", amt, bal);
}

static void withdraw(Account *acc) {
    char buf[128];
    printf("Enter amount to withdraw (e.g., 50 or 50.00): ");
    if (!read_line(buf, sizeof(buf))) return;
    long long cents = 0;
    if (!parse_amount_cents(buf, &cents) || cents <= 0) {
        printf("Invalid amount. Please try again.\n");
        return;
    }
    if (cents > acc->balance_cents) {
        printf("Insufficient funds. Current balance is lower than requested amount.\n");
        return;
    }
    acc->balance_cents -= cents;
    record_tx(acc, TX_WITHDRAW, cents);
    char amt[32]; format_amount(cents, amt, sizeof(amt));
    char bal[32]; format_amount(acc->balance_cents, bal, sizeof(bal));
    printf("Withdrew %s. New balance: %s\n", amt, bal);
}

static void view_transactions(const Account *acc) {
    if (acc->tx_count == 0) {
        printf("No transactions yet.\n");
        return;
    }
    printf("\n--- Transaction History (most recent last) ---\n");
    for (int i = 0; i < acc->tx_count; ++i) {
        const Transaction *tx = &acc->tx[i];
        char amt[32]; format_amount(tx->amount_cents, amt, sizeof(amt));
        char bal[32]; format_amount(tx->balance_after, bal, sizeof(bal));
        printf("[%3d] ", i + 1);
        print_timestamp(tx->timestamp);
        printf("  |  %s  %s  |  Balance: %s\n",
               tx->type == TX_DEPOSIT ? "DEPOSIT " : "WITHDRAW",
               amt,
               bal);
    }
    printf("---------------------------------------------\n\n");
}

static void view_balance(const Account *acc) {
    char bal[32]; format_amount(acc->balance_cents, bal, sizeof(bal));
    printf("Current balance: %s\n", bal);
}

// -------------------------------
// Menu/UI
// -------------------------------

static void print_menu(void) {
    printf("\n==============================\n");
    printf("          ATM MENU\n");
    printf("==============================\n");
    printf("1) Deposit\n");
    printf("2) Withdraw\n");
    printf("3) View transactions\n");
    printf("4) View balance\n");
    printf("0) Exit\n");
    printf("Select an option: ");
}

static void auto_load(Account *acc) {
    // Prefer JSON if available, else CSV
    if (file_exists(JSON_PATH) && load_from_json(acc, JSON_PATH)) {
        printf("Loaded history from %s.\n", JSON_PATH);
        return;
    }
    if (file_exists(CSV_PATH) && load_from_csv(acc, CSV_PATH)) {
        printf("Loaded history from %s.\n", CSV_PATH);
        return;
    }
    // No previous data
    acc->balance_cents = 0;
    acc->tx_count = 0;
}

static void auto_save(const Account *acc) {
    if (!save_to_csv(acc, CSV_PATH)) {
        fprintf(stderr, "Warning: failed to write %s\n", CSV_PATH);
    }
    if (!save_to_json(acc, JSON_PATH)) {
        fprintf(stderr, "Warning: failed to write %s\n", JSON_PATH);
    }
}

int main(void) {
    Account acc = {0};

    auto_load(&acc);

    char buf[64];
    int running = 1;
    while (running) {
        print_menu();
        if (!read_line(buf, sizeof(buf))) break;
        int choice = -1;
        if (!parse_int(buf, &choice)) {
            printf("Invalid input. Please enter a number from the menu.\n");
            continue;
        }

        switch (choice) {
            case 1: deposit(&acc); break;
            case 2: withdraw(&acc); break;
            case 3: view_transactions(&acc); break;
            case 4: view_balance(&acc); break;
            case 0: running = 0; break;
            default:
                printf("Unknown option. Please choose from the menu.\n");
                break;
        }
    }

    auto_save(&acc);
    printf("Goodbye! Data saved to %s and %s.\n", CSV_PATH, JSON_PATH);
    return 0;
}
