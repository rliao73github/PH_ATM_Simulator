# PH ATM Simulator

A simple, file-based ATM (Automated Teller Machine) simulator written in C. This terminal-based application allows users to perform basic banking operations with persistent data storage.

## Features

- **View Balance** - Check current account balance
- **Deposit** - Add funds to the account
- **Withdraw** - Remove funds from the account (with insufficient funds protection)
- **View Transactions** - Display complete transaction history with timestamps
- **Persistent Storage** - All data saved to files (survives program restarts)
- **Input Validation** - Prevents invalid operations and negative amounts
- **Detailed Logging** - Diagnostic logs for debugging and monitoring (visible in Docker)

## Technology Stack

- **Language**: C (C11 standard)
- **Compiler**: Clang / GCC
- **Containerization**: Docker
- **Platform**: Cross-platform (macOS, Linux via Docker)

## Project Structure

```
PH_ATM_Simulator/
├── PH_ATM_Simulator/
│   ├── main.c                  # Main source code
│   ├── atm_simulator           # Compiled executable (macOS)
│   ├── balance.txt             # Stores current balance
│   └── transactions.txt        # Stores transaction history
├── data/
│   ├── balance.txt             # Docker volume data
│   └── transactions.txt        # Docker volume data
├── Dockerfile                  # Docker configuration
├── DOCKER_README.md           # Docker-specific instructions
├── .dockerignore              # Docker build exclusions
└── README.md                  # This file
```

## Getting Started

### Prerequisites

**Option 1: Native (macOS)**
- Xcode Command Line Tools or Clang compiler

**Option 2: Docker**
- Docker Desktop or Docker Engine

### Installation & Running

#### Option 1: Compile and Run Natively (macOS)

1. **Clone the repository**
   ```bash
   git clone https://github.com/rliao73github/PH_ATM_Simulator.git
   cd PH_ATM_Simulator/PH_ATM_Simulator
   ```

2. **Compile the program**
   ```bash
   clang -o atm_simulator main.c
   ```

3. **Run the simulator**
   ```bash
   ./atm_simulator
   ```

#### Option 2: Run with Docker (Recommended for Linux/Cross-platform)

1. **Clone the repository**
   ```bash
   git clone https://github.com/rliao73github/PH_ATM_Simulator.git
   cd PH_ATM_Simulator
   ```

2. **Build the Docker image**
   ```bash
   docker build -t atm-simulator .
   ```

3. **Run the container**
   ```bash
   docker run -it --rm -v $(pwd)/data:/data atm-simulator
   ```

   This command:
   - `-it` - Enables interactive terminal mode
   - `--rm` - Automatically removes container after exit
   - `-v $(pwd)/data:/data` - Mounts local `data` folder for persistent storage

For more Docker options, see [DOCKER_README.md](DOCKER_README.md)

## Usage

Once the program starts, you'll see a menu with the following options:

```
========================================
              MAIN MENU
========================================
1. View Balance
2. Deposit
3. Withdraw
4. View Transactions
5. Exit
========================================
Enter your choice (default: 1):
```

- **Press Enter** without typing to automatically select option 1 (View Balance)
- **Type a number (1-5)** and press Enter to select an option
- **Enter amounts** as decimal numbers (e.g., 100.00 or 50.5)

### Example Session

```
Enter your choice: 2
========================================
              DEPOSIT
========================================
Current balance: $0.00
Enter amount to deposit: $500.00

Deposit successful!
Amount deposited: $500.00
New balance: $500.00
========================================

Enter your choice: 3
========================================
             WITHDRAWAL
========================================
Current balance: $500.00
Enter amount to withdraw: $100.00

Withdrawal successful!
Amount withdrawn: $100.00
New balance: $400.00
========================================

Enter your choice: 4
========================================
        TRANSACTION HISTORY
========================================
2025-10-25 14:30:15 | DEPOSIT    | $    500.00 | Balance: $500.00
2025-10-25 14:30:45 | WITHDRAW   | $    100.00 | Balance: $400.00
========================================
```

## Data Files

The simulator creates and maintains two files:

### `balance.txt`
Stores the current account balance as a decimal number.
```
400.00
```

### `transactions.txt`
Stores transaction history in the format:
```
YYYY-MM-DD HH:MM:SS | TYPE       | $    AMOUNT | Balance: $BALANCE
```

## Advanced Compilation Options

### Generate Intermediate Files
```bash
clang -save-temps -v main.c -o atm_simulator
```

This produces:
- `main.i` - Preprocessed source
- `main.bc` - LLVM bitcode
- `main.s` - Assembly code
- `main.o` - Object file
- `atm_simulator` - Final executable

### Optimized Build
```bash
clang -std=c11 -O2 -o atm_simulator main.c
```

### Debug Build
```bash
clang -g -o atm_simulator main.c
```

## Logging

The application writes diagnostic logs to `stderr`, which are captured by Docker but not displayed in the user interface. Logs include:

- Application startup/shutdown
- User menu selections
- Transaction details (amounts, balances)
- File operations
- Error conditions

**View logs in Docker Desktop**: Containers → Select running container → Logs tab

**View logs from terminal**:
```bash
docker run -it --rm -v $(pwd)/data:/data atm-simulator 2>&1 | tee atm.log
```

## Limitations

- **Single Account**: Only handles one account (no multi-user support)
- **No Authentication**: No PIN or password protection
- **Basic Validation**: Limited input validation and error handling
- **Text-based UI**: No graphical interface
- **Local Storage**: Files stored locally (no database or cloud sync)

## Future Enhancements

Potential improvements for future versions:
- Multiple account support
- PIN/password authentication
- Account transfer functionality
- Interest calculation
- Better error handling and input validation
- GUI interface
- Database integration
- Network/remote access capabilities

## Docker Desktop Note

**Important**: This interactive terminal application must be run from the command line with `-it` flags. Docker Desktop's GUI does not properly support real-time interactive input/output required by this application. Logs are visible in Docker Desktop's Logs tab after the container exits.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is open source and available for educational purposes.

## Author

Rong Liao

## Repository

https://github.com/rliao73github/PH_ATM_Simulator
