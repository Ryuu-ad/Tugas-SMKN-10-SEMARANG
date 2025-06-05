#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <sstream>

using namespace std;

const int MIN_PIN_LENGTH = 4;
const int MAX_PIN_LENGTH = 6;
const double MIN_INITIAL_BALANCE = 50000.0;
const double MAX_TRANSACTION_AMOUNT = 10000000.0;
const double MIN_TRANSACTION_AMOUNT = 10000.0;
const char ENCRYPTION_KEY = 'K';

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string encryptDecryptPIN(const string& pin, char key = ENCRYPTION_KEY) {
    string result = pin;
    for (char& c : result) {
        c ^= key;
    }
    return result;
}

bool isValidPIN(const string& pin) {
    if (pin.length() < MIN_PIN_LENGTH || pin.length() > MAX_PIN_LENGTH) {
        return false;
    }

    for (char c : pin) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool isValidAccountNumber(const string& accountNumber) {
    if (accountNumber.length() < 10 || accountNumber.length() > 15) {
        return false;
    }

    for (char c : accountNumber) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool isValidName(const string& name) {
    if (name.empty() || name.length() < 3 || name.length() > 50) {
        return false;
    }

    for (char c : name) {
        if (!isalpha(c) && c != ' ') {
            return false;
        }
    }
    return true;
}

double getValidAmount(const string& prompt) {
    double amount;
    while (true) {
        cout << prompt;
        if (cin >> amount && amount >= 0) {
            return amount;
        } else {
            cout << "Input tidak valid! Masukkan angka yang benar.\n";
            clearInputBuffer();
        }
    }
}

string formatCurrency(double amount) {
    stringstream ss;
    ss << fixed << setprecision(0) << "Rp " << amount;
    return ss.str();
}

struct Transaction {
    string type;
    double amount;
    string timestamp;
    string description;
    Transaction* next;

    Transaction(const string& t, double a, const string& desc)
        : type(t), amount(a), description(desc), next(nullptr) {
        time_t now = time(0);
        timestamp = ctime(&now);
        timestamp.pop_back();
    }
};

struct Account {
    string name;
    string accountNumber;
    string encryptedPIN;
    double balance;
    Transaction* transactions;
    Account* next;
    time_t createdAt;
    int loginAttempts;
    bool isLocked;

    Account() : balance(0), transactions(nullptr), next(nullptr),
                createdAt(time(0)), loginAttempts(0), isLocked(false) {}

    ~Account() {
        while (transactions != nullptr) {
            Transaction* temp = transactions;
            transactions = transactions->next;
            delete temp;
        }
    }
};

class ATMSystem {
private:
    Account* head;
    int totalAccounts;

public:
    ATMSystem() : head(nullptr), totalAccounts(0) {}

    ~ATMSystem() {
        cleanup();
    }

    Account* findAccount(const string& accountNumber) {
        Account* current = head;
        while (current != nullptr) {
            if (current->accountNumber == accountNumber) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    void addTransaction(Account* account, const string& type, double amount, const string& description) {
        Transaction* newTransaction = new Transaction(type, amount, description);
        newTransaction->next = account->transactions;
        account->transactions = newTransaction;
    }

    void registerAccount() {
        cout << "\n" << string(60, '=') << "\n";
        cout << "           REGISTRASI AKUN BARU" << "\n";
        cout << string(60, '=') << "\n";

        Account* newAccount = new Account;

        while (true) {
            cout << "Masukkan nama lengkap: ";
            getline(cin >> ws, newAccount->name);

            if (isValidName(newAccount->name)) {
                transform(newAccount->name.begin(), newAccount->name.end(),
                         newAccount->name.begin(), ::toupper);
                break;
            } else {
                cout << "Nama tidak valid! Nama harus 3-50 karakter dan hanya huruf.\n";
            }
        }

        while (true) {
            cout << "Masukkan nomor rekening (10-15 digit): ";
            cin >> newAccount->accountNumber;

            if (!isValidAccountNumber(newAccount->accountNumber)) {
                cout << "Nomor rekening tidak valid! Harus 10-15 digit angka.\n";
                continue;
            }

            if (findAccount(newAccount->accountNumber) != nullptr) {
                cout << "Nomor rekening sudah terdaftar! Gunakan nomor lain.\n";
                continue;
            }

            break;
        }

        string pin;
        while (true) {
            cout << "Masukkan PIN (" << MIN_PIN_LENGTH << "-" << MAX_PIN_LENGTH << " digit): ";
            cin >> pin;

            if (isValidPIN(pin)) {
                newAccount->encryptedPIN = encryptDecryptPIN(pin);
                break;
            } else {
                cout << "PIN tidak valid! Harus " << MIN_PIN_LENGTH << "-" << MAX_PIN_LENGTH << " digit angka.\n";
            }
        }

        while (true) {
            newAccount->balance = getValidAmount("Masukkan saldo awal (min " +
                                                formatCurrency(MIN_INITIAL_BALANCE) + "): ");

            if (newAccount->balance >= MIN_INITIAL_BALANCE) {
                break;
            } else {
                cout << "Saldo awal minimal " << formatCurrency(MIN_INITIAL_BALANCE) << "\n";
            }
        }

        newAccount->next = head;
        head = newAccount;
        totalAccounts++;

        addTransaction(newAccount, "DEPOSIT", newAccount->balance, "Saldo awal pembukaan rekening");

        cout << "\n" << string(60, '-') << "\n";
        cout << "AKUN BERHASIL TERDAFTAR!" << "\n";
        cout << "Nama: " << newAccount->name << "\n";
        cout << "No. Rekening: " << newAccount->accountNumber << "\n";
        cout << "Saldo Awal: " << formatCurrency(newAccount->balance) << "\n";
        cout << string(60, '-') << "\n";
    }

    Account* login() {
        cout << "\n" << string(60, '=') << "\n";
        cout << "                LOGIN SISTEM ATM" << "\n";
        cout << string(60, '=') << "\n";

        string accountNumber, pin;

        cout << "Masukkan nomor rekening: ";
        cin >> accountNumber;

        Account* user = findAccount(accountNumber);
        if (user == nullptr) {
            cout << "Nomor rekening tidak ditemukan!\n";
            return nullptr;
        }

        if (user->isLocked) {
            cout << "Akun Anda terkunci karena terlalu banyak percobaan login yang gagal!\n";
            cout << "Silakan hubungi customer service.\n";
            return nullptr;
        }

        cout << "Masukkan PIN: ";
        cin >> pin;

        if (user->encryptedPIN == encryptDecryptPIN(pin)) {
            user->loginAttempts = 0;
            cout << "\n" << string(60, '-') << "\n";
            cout << "LOGIN BERHASIL!" << "\n";
            cout << "Selamat datang, " << user->name << "\n";
            cout << string(60, '-') << "\n";
            return user;
        } else {
            user->loginAttempts++;
            cout << "PIN salah! Percobaan ke-" << user->loginAttempts << " dari 3\n";

            if (user->loginAttempts >= 3) {
                user->isLocked = true;
                cout << "Akun Anda telah dikunci karena 3 kali percobaan login gagal!\n";
            }
            return nullptr;
        }
    }

    void withdraw(Account* user) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "               TARIK TUNAI" << "\n";
        cout << string(60, '=') << "\n";

        cout << "Saldo saat ini: " << formatCurrency(user->balance) << "\n\n";

        double amount = getValidAmount("Masukkan jumlah yang ingin ditarik: ");

        if (amount < MIN_TRANSACTION_AMOUNT) {
            cout << "Jumlah penarikan minimal " << formatCurrency(MIN_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        if (amount > MAX_TRANSACTION_AMOUNT) {
            cout << "Jumlah penarikan maksimal " << formatCurrency(MAX_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        if (amount > user->balance) {
            cout << "Saldo tidak mencukupi!\n";
            cout << "Saldo tersedia: " << formatCurrency(user->balance) << "\n";
            return;
        }

        user->balance -= amount;
        addTransaction(user, "WITHDRAW", amount, "Penarikan tunai");

        cout << "\n" << string(60, '-') << "\n";
        cout << "PENARIKAN BERHASIL!" << "\n";
        cout << "Jumlah ditarik: " << formatCurrency(amount) << "\n";
        cout << "Saldo tersisa: " << formatCurrency(user->balance) << "\n";
        cout << string(60, '-') << "\n";
    }

    void deposit(Account* user) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "               SETOR TUNAI" << "\n";
        cout << string(60, '=') << "\n";

        cout << "Saldo saat ini: " << formatCurrency(user->balance) << "\n\n";

        double amount = getValidAmount("Masukkan jumlah yang ingin disetor: ");

        if (amount < MIN_TRANSACTION_AMOUNT) {
            cout << "Jumlah setoran minimal " << formatCurrency(MIN_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        if (amount > MAX_TRANSACTION_AMOUNT) {
            cout << "Jumlah setoran maksimal " << formatCurrency(MAX_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        user->balance += amount;
        addTransaction(user, "DEPOSIT", amount, "Setoran tunai");

        cout << "\n" << string(60, '-') << "\n";
        cout << "SETORAN BERHASIL!" << "\n";
        cout << "Jumlah disetor: " << formatCurrency(amount) << "\n";
        cout << "Saldo sekarang: " << formatCurrency(user->balance) << "\n";
        cout << string(60, '-') << "\n";
    }

    void transfer(Account* user) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "               TRANSFER DANA" << "\n";
        cout << string(60, '=') << "\n";

        cout << "Saldo saat ini: " << formatCurrency(user->balance) << "\n\n";

        string targetAccountNumber;
        cout << "Masukkan nomor rekening tujuan: ";
        cin >> targetAccountNumber;

        Account* target = findAccount(targetAccountNumber);
        if (target == nullptr) {
            cout << "Rekening tujuan tidak ditemukan!\n";
            return;
        }

        if (target == user) {
            cout << "Tidak dapat transfer ke rekening sendiri!\n";
            return;
        }

        cout << "Rekening tujuan: " << target->name << "\n";
        cout << "Konfirmasi transfer ke rekening ini? (y/n): ";
        char confirm;
        cin >> confirm;

        if (confirm != 'y' && confirm != 'Y') {
            cout << "Transfer dibatalkan.\n";
            return;
        }

        double amount = getValidAmount("Masukkan jumlah transfer: ");

        if (amount < MIN_TRANSACTION_AMOUNT) {
            cout << "Jumlah transfer minimal " << formatCurrency(MIN_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        if (amount > MAX_TRANSACTION_AMOUNT) {
            cout << "Jumlah transfer maksimal " << formatCurrency(MAX_TRANSACTION_AMOUNT) << "\n";
            return;
        }

        if (amount > user->balance) {
            cout << "Saldo tidak mencukupi untuk transfer!\n";
            cout << "Saldo tersedia: " << formatCurrency(user->balance) << "\n";
            return;
        }

        user->balance -= amount;
        target->balance += amount;

        addTransaction(user, "TRANSFER", amount, "Transfer ke " + target->name + " (" + target->accountNumber + ")");

        addTransaction(target, "RECEIVE", amount, "Transfer dari " + user->name + " (" + user->accountNumber + ")");

        cout << "\n" << string(60, '-') << "\n";
        cout << "TRANSFER BERHASIL!" << "\n";
        cout << "Tujuan: " << target->name << "\n";
        cout << "Jumlah: " << formatCurrency(amount) << "\n";
        cout << "Saldo tersisa: " << formatCurrency(user->balance) << "\n";
        cout << string(60, '-') << "\n";
    }

    void checkBalance(Account* user) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "            INFORMASI AKUN" << "\n";
        cout << string(60, '=') << "\n";

        cout << "Nama: " << user->name << "\n";
        cout << "No. Rekening: " << user->accountNumber << "\n";
        cout << "Saldo: " << formatCurrency(user->balance) << "\n";
        cout << "Akun dibuat: " << ctime(&user->createdAt);

        cout << string(60, '-') << "\n";
    }

    void viewTransactionHistory(Account* user) {
        cout << "\n" << string(80, '=') << "\n";
        cout << "                    RIWAYAT TRANSAKSI" << "\n";
        cout << string(80, '=') << "\n";

        if (user->transactions == nullptr) {
            cout << "Belum ada transaksi.\n";
            return;
        }

        cout << left << setw(12) << "JENIS" << setw(15) << "JUMLAH"
             << setw(25) << "WAKTU" << "KETERANGAN\n";
        cout << string(80, '-') << "\n";

        Transaction* current = user->transactions;
        int count = 0;
        while (current != nullptr && count < 10) {
            cout << left << setw(12) << current->type
                 << setw(15) << formatCurrency(current->amount)
                 << setw(25) << current->timestamp.substr(0, 20)
                 << current->description << "\n";
            current = current->next;
            count++;
        }

        cout << string(80, '-') << "\n";
    }

    void changePIN(Account* user) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "               GANTI PIN" << "\n";
        cout << string(60, '=') << "\n";

        string oldPIN, newPIN, confirmPIN;

        cout << "Masukkan PIN lama: ";
        cin >> oldPIN;

        if (user->encryptedPIN != encryptDecryptPIN(oldPIN)) {
            cout << "PIN lama salah!\n";
            return;
        }

        while (true) {
            cout << "Masukkan PIN baru (" << MIN_PIN_LENGTH << "-" << MAX_PIN_LENGTH << " digit): ";
            cin >> newPIN;

            if (!isValidPIN(newPIN)) {
                cout << "PIN tidak valid! Harus " << MIN_PIN_LENGTH << "-" << MAX_PIN_LENGTH << " digit angka.\n";
                continue;
            }

            cout << "Konfirmasi PIN baru: ";
            cin >> confirmPIN;

            if (newPIN == confirmPIN) {
                user->encryptedPIN = encryptDecryptPIN(newPIN);
                cout << "\nPIN berhasil diubah!\n";
                return;
            } else {
                cout << "PIN tidak cocok! Coba lagi.\n";
            }
        }
    }

    void userMenu(Account* user) {
        int choice;
        do {
            cout << "\n" << string(60, '=') << "\n";
            cout << "               MENU UTAMA ATM" << "\n";
            cout << string(60, '=') << "\n";
            cout << "1. Tarik Tunai\n";
            cout << "2. Setor Tunai\n";
            cout << "3. Transfer Dana\n";
            cout << "4. Cek Saldo\n";
            cout << "5. Riwayat Transaksi\n";
            cout << "6. Ganti PIN\n";
            cout << "7. Logout\n";
            cout << string(60, '-') << "\n";
            cout << "Pilihan Anda (1-7): ";

            if (!(cin >> choice)) {
                clearInputBuffer();
                cout << "Input tidak valid! Masukkan angka 1-7.\n";
                continue;
            }

            switch (choice) {
                case 1: withdraw(user); break;
                case 2: deposit(user); break;
                case 3: transfer(user); break;
                case 4: checkBalance(user); break;
                case 5: viewTransactionHistory(user); break;
                case 6: changePIN(user); break;
                case 7:
                    cout << "\nLogout berhasil! Terima kasih telah menggunakan layanan kami.\n";
                    break;
                default:
                    cout << "Pilihan tidak valid! Masukkan angka 1-7.\n";
            }

            if (choice != 7) {
                cout << "\nTekan Enter untuk melanjutkan...";
                cin.ignore();
                cin.get();
            }

        } while (choice != 7);
    }

    void showSystemStats() {
        cout << "\n" << string(60, '=') << "\n";
        cout << "            STATISTIK SISTEM" << "\n";
        cout << string(60, '=') << "\n";

        cout << "Total akun terdaftar: " << totalAccounts << "\n";

        double totalBalance = 0;
        Account* current = head;
        while (current != nullptr) {
            totalBalance += current->balance;
            current = current->next;
        }

        cout << "Total dana dalam sistem: " << formatCurrency(totalBalance) << "\n";
        cout << string(60, '-') << "\n";
    }

    void mainMenu() {
        int choice;
        do {
            cout << "\n" << string(70, '=') << "\n";
            cout << "                    SELAMAT DATANG DI ATM SYSTEM" << "\n";
            cout << "                         Version 2.0" << "\n";
            cout << string(70, '=') << "\n";
            cout << "1. Registrasi Akun Baru\n";
            cout << "2. Login ke Akun\n";
            cout << "3. Lihat Statistik Sistem\n";
            cout << "4. Keluar\n";
            cout << string(70, '-') << "\n";
            cout << "Pilihan Anda (1-4): ";

            if (!(cin >> choice)) {
                clearInputBuffer();
                cout << "Input tidak valid! Masukkan angka 1-4.\n";
                continue;
            }

            switch (choice) {
                case 1:
                    registerAccount();
                    break;
                case 2: {
                    Account* user = login();
                    if (user != nullptr) {
                        userMenu(user);
                    }
                    break;
                }
                case 3:
                    showSystemStats();
                    break;
                case 4:
                    cout << "\n" << string(70, '-') << "\n";
                    cout << "Terima kasih telah menggunakan ATM System!\n";
                    cout << "Program ditutup dengan aman.\n";
                    cout << string(70, '-') << "\n";
                    break;
                default:
                    cout << "Pilihan tidak valid! Masukkan angka 1-4.\n";
            }

            if (choice != 4 && choice >= 1 && choice <= 3) {
                cout << "\nTekan Enter untuk kembali ke menu utama...";
                cin.ignore();
                cin.get();
            }

        } while (choice != 4);
    }

    void cleanup() {
        while (head != nullptr) {
            Account* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

int main() {
    cout << fixed << setprecision(0);

    cout << "\n" << string(70, '*') << "\n";
    cout << "*                 SISTEM MANAJEMEN ATM SEDERHANA                     *\n";
    cout << "*               Pengembang : Miyuu, LagiSikma, s3d5p                 *\n";
    cout << "*                Menggunakan Linked List & Enkripsi                  *\n";
    cout << string(70, '*') << "\n";

    ATMSystem atmSystem;
    atmSystem.mainMenu();

    return 0;
}
