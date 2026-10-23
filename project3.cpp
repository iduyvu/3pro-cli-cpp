#include <iostream>
#include <cstring>
#include <iomanip>

#ifdef _WIN32
#include <conio.h> // Cho Windows
#else
#include <termios.h> // Cho Linux/macOS
#include <unistd.h>
// Hàm mô phỏng _getch() trên Linux
char _getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror ("tcsetattr ~ICANON");
    return buf;
}
#endif

using namespace std;

// ==================== CẤU TRÚC DỮ LIỆU ====================

// Cấu trúc Nút Sản Phẩm trong Danh sách liên kết vòng (Circular Linked List)
// Khai báo trước
struct Transaction;

// Cấu trúc Nút Sản Phẩm trong Danh sách liên kết vòng (Circular Linked List)
struct Product {
    char id[20];
    char name[50];
    int sales;       // Số lượng bán được (Dùng cho Heap Sort)
    Product* next;   // Con trỏ trỏ tới sản phẩm tiếp theo trong vòng

    Transaction* txHead; // Con trỏ quản lý chuỗi giao dịch của Product (cho ma trận thưa)

    Product(const char* pId, const char* pName, int pSales) {
        strncpy(id, pId, 19); id[19] = '\0';
        strncpy(name, pName, 49); name[49] = '\0';
        sales = pSales;
        next = nullptr;
        txHead = nullptr;
    }
};

// Cấu trúc Danh mục quản lý Danh sách vòng Sản phẩm
struct Category {
    char name[50];
    Product* head; // Con trỏ đầu danh sách vòng

    Category(const char* cName) {
        strncpy(name, cName, 49); name[49] = '\0';
        head = nullptr;
    }
};

// Cấu trúc Nút Giao Dịch trong Danh sách ma trận thưa 2 chiều
struct Transaction {
    char txId[20];
    char userId[20];
    char productId[20];

    // 2 con trỏ ma trận thưa
    Transaction* nextUserTx;    // Giao dịch tiếp theo của cùng 1 User
    Transaction* nextProductTx; // Giao dịch tiếp theo của cùng 1 Product

    Transaction(const char* tId, const char* uId, const char* pId) {
        strncpy(txId, tId, 19); txId[19] = '\0';
        strncpy(userId, uId, 19); userId[19] = '\0';
        strncpy(productId, pId, 19); productId[19] = '\0';
        nextUserTx = nullptr;
        nextProductTx = nullptr;
    }
};

// Cấu trúc Nút User trong Cây AVL
struct UserNode {
    char id[20];
    char email[50];
    char password[50];

    UserNode* left;
    UserNode* right;
    int height;

    Transaction* txHead; // Con trỏ tới chuỗi giao dịch của User này

    UserNode(const char* uId, const char* uEmail, const char* uPassword) {
        strncpy(id, uId, 19); id[19] = '\0';
        strncpy(email, uEmail, 49); email[49] = '\0';
        strncpy(password, uPassword, 49); password[49] = '\0';
        left = right = nullptr;
        height = 1;
        txHead = nullptr;
    }
};

// ==================== BIẾN TOÀN CỤC ====================
UserNode* userTreeRoot = nullptr;      // Gốc của cây AVL Users
Category* categories[20];              // Mảng con trỏ quản lý Categories
int categoryCount = 0;

Product* topProductsHeap[10];          // Min-Heap (chứa TOP 10)
int heapSize = 0;


// ==================== AVL TREE (QUẢN LÝ USER) ====================
// Lấy chiều cao của node
int getHeight(UserNode* node) {
    if (node == nullptr) return 0;
    return node->height;
}

// Lấy độ lệch (Balance Factor)
int getBalance(UserNode* node) {
    if (node == nullptr) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Xoay phải thủ công bằng con trỏ
UserNode* rightRotate(UserNode* y) {
    UserNode* x = y->left;
    UserNode* T2 = x->right;

    // Thực hiện xoay
    x->right = y;
    y->left = T2;

    // Cập nhật chiều cao
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

    return x; // Trả về gốc mới
}

// Xoay trái thủ công bằng con trỏ
UserNode* leftRotate(UserNode* x) {
    UserNode* y = x->right;
    UserNode* T2 = y->left;

    // Thực hiện xoay
    y->left = x;
    x->right = T2;

    // Cập nhật chiều cao
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

    return y; // Trả về gốc mới
}

// Chèn User vào AVL Tree
UserNode* insertUser(UserNode* node, const char* id, const char* email, const char* password) {
    if (node == nullptr) return new UserNode(id, email, password);

    if (strcmp(id, node->id) < 0)
        node->left = insertUser(node->left, id, email, password);
    else if (strcmp(id, node->id) > 0)
        node->right = insertUser(node->right, id, email, password);
    else
        return node; // Không cho phép trùng ID

    // Cập nhật chiều cao node hiện tại
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));

    // Kiểm tra cân bằng
    int balance = getBalance(node);

    // Các trường hợp xoay
    // Left Left
    if (balance > 1 && strcmp(id, node->left->id) < 0)
        return rightRotate(node);
    // Right Right
    if (balance < -1 && strcmp(id, node->right->id) > 0)
        return leftRotate(node);
    // Left Right
    if (balance > 1 && strcmp(id, node->left->id) > 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    // Right Left
    if (balance < -1 && strcmp(id, node->right->id) < 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

UserNode* searchUser(UserNode* node, const char* id) {
    if (node == nullptr || strcmp(node->id, id) == 0) return node;
    if (strcmp(id, node->id) < 0) return searchUser(node->left, id);
    return searchUser(node->right, id);
}

// ==================== CÁC HÀM XỬ LÝ ĐẦU VÀO ====================
// Phân tích Email không dùng Regex (chỉ dùng con trỏ)
bool validateEmail(const char* email) {
    const char* ptr = email;
    bool hasAt = false;
    bool hasDotAfterAt = false;

    while (*ptr != '\0') {
        if (*ptr == '@') {
            if (hasAt) return false; // Không được có 2 chữ @
            hasAt = true;
        } else if (*ptr == '.' && hasAt) {
            hasDotAfterAt = true;
        }
        ptr++;
    }
    return (hasAt && hasDotAfterAt);
}

// Nhập mật khẩu ẩn bằng dấu * (Ghi đè thủ công)
void inputPassword(char* pwd, int maxLen) {
    int i = 0;
    char ch;
    while (i < maxLen - 1) {
        ch = _getch();
        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == '\b' || ch == 127) { // Xử lý phím Backspace
            if (i > 0) {
                cout << "\b \b";
                i--;
            }
        } else {
            pwd[i++] = ch;
            cout << '*'; // Hiển thị dấu sao
        }
    }
    pwd[i] = '\0';
    cout << "\n";
}


// ==================== QUẢN LÝ DANH MỤC VÀ SẢN PHẨM (CIRCULAR LINKED LIST) ====================
void addCategory(const char* name) {
    if (categoryCount < 20) {
        categories[categoryCount++] = new Category(name);
        cout << "Da them danh muc: " << name << "\n";
    }
}

// Thêm sản phẩm vào danh sách liên kết vòng
Product* searchProduct(const char* id) {
    for (int i = 0; i < categoryCount; i++) {
        Category* cat = categories[i];
        if (cat->head != nullptr) {
            Product* current = cat->head;
            do {
                if (strcmp(current->id, id) == 0) return current;
                current = current->next;
            } while (current != cat->head);
        }
    }
    return nullptr;
}

// Thêm sản phẩm vào danh sách liên kết vòng
void addProduct(int catIndex, const char* id, const char* name, int sales) {
    if (catIndex < 0 || catIndex >= categoryCount) return;

    Product* newProduct = new Product(id, name, sales);
    Category* cat = categories[catIndex];

    if (cat->head == nullptr) {
        cat->head = newProduct;
        newProduct->next = newProduct; // Tự trỏ vào chính nó tạo vòng
    } else {
        Product* current = cat->head;
        while (current->next != cat->head) {
            current = current->next;
        }
        current->next = newProduct;
        newProduct->next = cat->head; // Nối đuôi vào đầu
    }
    cout << "Da them san pham: " << name << "\n";
}


// ==================== DUY TRÌ MIN-HEAP THEO THỜI GIAN THỰC (TOP 10 SẢN PHẨM) ====================

// Hàm Heapify cho Min-Heap
void minHeapify(int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapSize && topProductsHeap[left]->sales < topProductsHeap[smallest]->sales)
        smallest = left;
    if (right < heapSize && topProductsHeap[right]->sales < topProductsHeap[smallest]->sales)
        smallest = right;

    if (smallest != i) {
        Product* temp = topProductsHeap[i];
        topProductsHeap[i] = topProductsHeap[smallest];
        topProductsHeap[smallest] = temp;
        minHeapify(smallest);
    }
}

// Cập nhật Min-Heap khi có sản phẩm mới hoặc sản phẩm thay đổi doanh số
void updateTopProducts(Product* prod) {
    // Kiểm tra xem sản phẩm đã có trong heap chưa
    int index = -1;
    for (int i = 0; i < heapSize; i++) {
        if (topProductsHeap[i] == prod) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        // Sản phẩm đã có trong heap, doanh số vừa tăng lên
        // Trong Min-Heap, nếu giá trị tăng, nó chỉ có thể "chìm xuống"
        minHeapify(index);
    } else {
        // Sản phẩm chưa có trong heap
        if (heapSize < 10) {
            // Heap chưa đầy, thêm vào cuối rồi "nổi lên"
            int i = heapSize;
            topProductsHeap[i] = prod;
            heapSize++;

            // Sift up
            while (i != 0 && topProductsHeap[(i - 1) / 2]->sales > topProductsHeap[i]->sales) {
                Product* temp = topProductsHeap[i];
                topProductsHeap[i] = topProductsHeap[(i - 1) / 2];
                topProductsHeap[(i - 1) / 2] = temp;
                i = (i - 1) / 2;
            }
        } else if (prod->sales > topProductsHeap[0]->sales) {
            // Heap đã đầy, nhưng sản phẩm này bán chạy hơn sản phẩm tệ nhất trong Top 10 (Gốc Min-Heap)
            topProductsHeap[0] = prod; // Thay thế phần tử gốc
            minHeapify(0); // Cho nó "chìm xuống" đúng vị trí
        }
    }
}

void showTopProducts() {
    cout << "\n=== TOP " << heapSize << " SAN PHAM BAN CHAY NHAT (REAL-TIME MIN-HEAP) ===\n";
    // Mặc dù là Min-Heap, nhưng ta sẽ in ra toàn bộ phần tử trong Heap.
    // Để in giảm dần (đẹp mắt), chúng ta có thể copy ra mảng phụ và sắp xếp, hoặc in thô:

    // Copy ra mảng phụ để hiển thị mà không phá hỏng Heap thật
    Product* tempArr[10];
    for (int i = 0; i < heapSize; i++) tempArr[i] = topProductsHeap[i];

    // Đơn giản hóa: Sắp xếp thủ công mảng phụ (tối đa 10 phần tử nên rất nhanh)
    for (int i = 0; i < heapSize - 1; i++) {
        for (int j = i + 1; j < heapSize; j++) {
            if (tempArr[i]->sales < tempArr[j]->sales) {
                Product* t = tempArr[i];
                tempArr[i] = tempArr[j];
                tempArr[j] = t;
            }
        }
    }

    for (int i = 0; i < heapSize; i++) {
        cout << tempArr[i]->id << " | " << tempArr[i]->name << " | Sales: " << tempArr[i]->sales << "\n";
    }
}


// ==================== QUẢN LÝ GIAO DỊCH (2-WAY SPARSE MATRIX) ====================
void addTransaction(const char* txId, const char* uId, const char* pId) {
    UserNode* user = searchUser(userTreeRoot, uId);
    if (!user) {
        cout << "Khong tim thay User!\n";
        return;
    }

    Product* product = searchProduct(pId);
    if (!product) {
        cout << "Khong tim thay Product!\n";
        return;
    }

    Transaction* newTx = new Transaction(txId, uId, pId);

    // Thêm vào chuỗi User Tx (Đầu chuỗi)
    newTx->nextUserTx = user->txHead;
    user->txHead = newTx;

    // Thêm vào chuỗi Product Tx (Đầu chuỗi) -> Đúng bản chất Ma Trận Thưa (Sparse Matrix)
    newTx->nextProductTx = product->txHead;
    product->txHead = newTx;

    // Tăng doanh số bán hàng và cập nhật Min-Heap Real-Time
    product->sales++;
    updateTopProducts(product);

    cout << "Da tao giao dich: " << txId << "\n";
}

// Xóa giao dịch mà không làm đứt liên kết (Không tạo Dangling Pointer)
void deleteTransactionSafe(const char* txId) {
    // 1. Tìm Transaction (Duyệt qua cây User)
    Transaction* targetTx = nullptr;
    UserNode* targetUser = nullptr;

    // Hàm phụ đệ quy để duyệt cây AVL tìm User chứa Transaction
    struct TxFinder {
        static void find(UserNode* node, const char* tId, Transaction*& foundTx, UserNode*& fUser) {
            if (!node || foundTx) return;
            Transaction* tx = node->txHead;
            while (tx) {
                if (strcmp(tx->txId, tId) == 0) {
                    foundTx = tx;
                    fUser = node;
                    return;
                }
                tx = tx->nextUserTx;
            }
            find(node->left, tId, foundTx, fUser);
            find(node->right, tId, foundTx, fUser);
        }
    };

    TxFinder::find(userTreeRoot, txId, targetTx, targetUser);

    if (!targetTx) {
        cout << "Khong tim thay giao dich de xoa.\n";
        return;
    }

    // 2. Xóa khỏi chuỗi của User
    if (targetUser) {
        Transaction* uPrev = nullptr;
        Transaction* uCurr = targetUser->txHead;
        while (uCurr != nullptr && uCurr != targetTx) {
            uPrev = uCurr;
            uCurr = uCurr->nextUserTx;
        }
        if (uCurr) {
            if (uPrev) uPrev->nextUserTx = uCurr->nextUserTx;
            else targetUser->txHead = uCurr->nextUserTx;
        }
    }

    // 3. Xóa khỏi chuỗi của Product
    Product* product = searchProduct(targetTx->productId);
    if (product) {
        Transaction* pPrev = nullptr;
        Transaction* pCurr = product->txHead;
        while (pCurr != nullptr && pCurr != targetTx) {
            pPrev = pCurr;
            pCurr = pCurr->nextProductTx;
        }
        if (pCurr) {
            if (pPrev) pPrev->nextProductTx = pCurr->nextProductTx;
            else product->txHead = pCurr->nextProductTx;
        }
    }

    delete targetTx; // Giải phóng an toàn
    cout << "Da xoa giao dich an toan, khong co con tro treo.\n";
}

// ==================== HÀM TIỆN ÍCH DỌN DẸP ====================
void deleteUserTreeAndTransactions(UserNode* node) {
    if (!node) return;
    deleteUserTreeAndTransactions(node->left);
    deleteUserTreeAndTransactions(node->right);

    // Xóa tất cả các giao dịch của User này
    Transaction* currTx = node->txHead;
    while (currTx) {
        Transaction* nextTx = currTx->nextUserTx;
        delete currTx;
        currTx = nextTx;
    }

    delete node;
}

void cleanUp() {
    // 1. Dọn dẹp User Tree VÀ Giao Dịch
    // (Vì mọi Giao dịch đều thuộc về một User, việc duyệt User Tree sẽ giải phóng sạch Transaction)
    deleteUserTreeAndTransactions(userTreeRoot);

    // 2. Dọn dẹp Danh mục và Sản phẩm (Circular Linked List)
    for (int i = 0; i < categoryCount; i++) {
        Category* cat = categories[i];
        if (cat->head != nullptr) {
            Product* current = cat->head;
            Product* nextProd = nullptr;
            do {
                nextProd = current->next;
                delete current;
                current = nextProd;
            } while (current != cat->head);
        }
        delete cat;
    }

    cout << "Da don dep bo nho an toan.\n";
}


int main() {
    addCategory("Dien Tu");
    addCategory("Thoi Trang");

    addProduct(0, "P01", "Laptop Gaming", 150);
    addProduct(0, "P02", "Chuot Khong Day", 300);
    addProduct(1, "P03", "Ao Thun", 500);

    userTreeRoot = insertUser(userTreeRoot, "U01", "test@mail.com", "pass123");

    int choice;
    do {
        cout << "\n=== E-COMMERCE ENGINE ===\n";
        cout << "1. Dang ky User (Kiem tra Email & Pass an)\n";
        cout << "2. Tao Giao Dich\n";
        cout << "3. Xoa Giao Dich An Toan (Chong con tro treo)\n";
        cout << "4. Hien thi Top 10 San pham (Heap Sort)\n";
        cout << "0. Thoat va Don dep\n";
        cout << "Lua chon: ";
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(10000, '\n'); continue;
        }

        switch (choice) {
            case 1: {
                char id[20], email[50], pwd[50];
                cout << "ID: "; cin >> setw(20) >> id;
                cout << "Email (Phai co @ va .): "; cin >> setw(50) >> email;
                if (!validateEmail(email)) {
                    cout << "Loi: Email khong hop le.\n";
                    break;
                }
                cout << "Password: ";
                inputPassword(pwd, 50);
                userTreeRoot = insertUser(userTreeRoot, id, email, pwd);
                cout << "Da dang ky User thanh cong vao AVL Tree!\n";
                break;
            }
            case 2: {
                char tId[20], uId[20], pId[20];
                cout << "Tx ID: "; cin >> setw(20) >> tId;
                cout << "User ID: "; cin >> setw(20) >> uId;
                cout << "Product ID: "; cin >> setw(20) >> pId;
                addTransaction(tId, uId, pId);
                break;
            }
            case 3: {
                char tId[20];
                cout << "Tx ID can xoa: "; cin >> setw(20) >> tId;
                deleteTransactionSafe(tId);
                break;
            }
            case 4:
                showTopProducts();
                break;
            case 0:
                cleanUp();
                break;
            default:
                cout << "Khong hop le.\n";
        }
    } while(choice != 0);

    return 0;
}
