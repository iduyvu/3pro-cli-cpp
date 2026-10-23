#include <iostream>
#include <cstring>
#include <iomanip>

using namespace std;

// Cấu trúc Trạm Dừng trong cây BST và danh sách đa liên kết
struct StationNode {
    char id[20];            // ID của trạm
    char name[50];          // Tên trạm (đơn giản hóa)

    // Con trỏ dùng cho danh sách đa liên kết (Multi-linked list) 3 chiều
    StationNode* nextStation;   // Trạm tiếp theo trên cùng tuyến
    StationNode* prevStation;   // Trạm trước đó trên cùng tuyến
    StationNode* transferLink;  // Trạm trung chuyển (nếu là ga giao cắt)

    // Con trỏ dùng cho Cây Nhị Phân Tìm Kiếm (BST) để tra cứu nhanh O(log N)
    StationNode* left;
    StationNode* right;

    StationNode(const char* stationId, const char* stationName) {
        strncpy(id, stationId, 19);
        id[19] = '\0';
        strncpy(name, stationName, 49);
        name[49] = '\0';

        nextStation = nullptr;
        prevStation = nullptr;
        transferLink = nullptr;

        left = nullptr;
        right = nullptr;
    }
};

// Cấu trúc Tuyến Đường (Route) quản lý các trạm
struct Route {
    char routeName[30];
    int passengerTraffic;       // Lưu lượng khách
    StationNode* headStation;   // Trạm đầu tuyến

    Route(const char* name, int traffic) {
        strncpy(routeName, name, 29);
        routeName[29] = '\0';
        passengerTraffic = traffic;
        headStation = nullptr;
    }
};

// Biến toàn cục
StationNode* bstRoot = nullptr; // Gốc của cây BST chứa TẤT CẢ các trạm để tìm kiếm nhanh
Route* routes[50];              // Mảng con trỏ chứa các tuyến đường
int routeCount = 0;


// ==================== CÂY NHỊ PHÂN TÌM KIẾM (BST) ====================
// Thêm node vào BST (dựa theo ID)
StationNode* insertBST(StationNode* root, StationNode* newNode) {
    if (root == nullptr) return newNode;

    // So sánh chuỗi ID để quyết định nhánh trái hay phải
    if (strcmp(newNode->id, root->id) < 0) {
        root->left = insertBST(root->left, newNode);
    } else if (strcmp(newNode->id, root->id) > 0) {
        root->right = insertBST(root->right, newNode);
    }
    return root;
}

// Tìm kiếm trạm trong BST (Độ phức tạp O(log N))
StationNode* searchBST(StationNode* root, const char* targetId) {
    if (root == nullptr || strcmp(root->id, targetId) == 0) {
        return root;
    }

    if (strcmp(targetId, root->id) < 0) {
        return searchBST(root->left, targetId);
    }
    return searchBST(root->right, targetId);
}

// Giải phóng bộ nhớ toàn bộ BST
void deleteBST(StationNode* root) {
    if (root == nullptr) return;
    deleteBST(root->left);
    deleteBST(root->right);
    delete root; // Giải phóng node trạm
}
// =====================================================================


// Phân tích thời gian HH:MM bằng con trỏ (Tránh tràn bộ đệm)
bool parseTime(const char* timeStr, int* hour, int* minute) {
    const char* ptr = timeStr;

    // Đọc giờ (Hour)
    *hour = 0;
    while (*ptr >= '0' && *ptr <= '9') {
        *hour = (*hour * 10) + (*ptr - '0');
        ptr++;
    }

    if (*ptr != ':') return false; // Phải có dấu :
    ptr++; // Bỏ qua dấu :

    // Đọc phút (Minute)
    *minute = 0;
    int minLength = 0;
    while (*ptr >= '0' && *ptr <= '9') {
        *minute = (*minute * 10) + (*ptr - '0');
        ptr++;
        minLength++;
    }

    if (minLength != 2) return false;
    if (*hour < 0 || *hour > 23 || *minute < 0 || *minute > 59) return false;

    return true;
}

// Thêm một tuyến đường mới
void addRoute(const char* name, int traffic) {
    if (routeCount < 50) {
        routes[routeCount] = new Route(name, traffic);
        routeCount++;
        cout << "Da them tuyen duong: " << name << "\n";
    } else {
        cout << "Mang tuyen duong da day!\n";
    }
}

// Thêm một trạm vào tuyến đường (và đồng thời lưu vào BST)
void addStationToRoute(int routeIndex, const char* id, const char* name) {
    if (routeIndex < 0 || routeIndex >= routeCount) {
        cout << "Tuyen duong khong hop le!\n";
        return;
    }

    // Kiểm tra xem trạm đã tồn tại trong BST chưa
    StationNode* existing = searchBST(bstRoot, id);
    if (existing != nullptr) {
        cout << "Loi: Tram voi ID " << id << " da ton tai!\n";
        return;
    }

    StationNode* newStation = new StationNode(id, name);
    bstRoot = insertBST(bstRoot, newStation); // Lưu vào BST để quản lý

    Route* route = routes[routeIndex];
    if (route->headStation == nullptr) {
        route->headStation = newStation;
    } else {
        // Tìm trạm cuối tuyến và nối vào
        StationNode* current = route->headStation;
        while (current->nextStation != nullptr) {
            current = current->nextStation;
        }
        current->nextStation = newStation;
        newStation->prevStation = current; // Liên kết 2 chiều
    }
    cout << "Da them tram " << name << " vao tuyen " << route->routeName << "\n";
}

// Thiết lập liên kết trung chuyển (transferLink) giữa 2 trạm
void setTransferLink(const char* id1, const char* id2) {
    StationNode* s1 = searchBST(bstRoot, id1);
    StationNode* s2 = searchBST(bstRoot, id2);

    if (s1 != nullptr && s2 != nullptr) {
        s1->transferLink = s2;
        s2->transferLink = s1; // Trung chuyển 2 chiều
        cout << "Da tao lien ket trung chuyen giua " << s1->name << " va " << s2->name << "\n";
    } else {
        cout << "Mot hoac ca hai tram khong ton tai!\n";
    }
}

// ==================== QUICKSORT TRÊN MẢNG CON TRỎ ====================
// Phân hoạch mảng (Partition) theo lưu lượng khách
int partition(Route* arr[], int low, int high) {
    int pivot = arr[high]->passengerTraffic;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        // Sắp xếp giảm dần (tuyến đông khách nhất lên đầu)
        if (arr[j]->passengerTraffic >= pivot) {
            i++;
            // Hoán vị mảng con trỏ (chỉ hoán vị con trỏ Route*, không đụng tới dữ liệu)
            Route* temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    // Hoán vị pivot vào đúng vị trí
    Route* temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

// Thuật toán QuickSort
void quickSortRoutes(Route* arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSortRoutes(arr, low, pi - 1);
        quickSortRoutes(arr, pi + 1, high);
    }
}
// =====================================================================


// Hiển thị danh sách các tuyến đường
void displayRoutes() {
    cout << "\n=== DANH SACH TUYEN DUONG ===\n";
    for (int i = 0; i < routeCount; i++) {
        cout << "[" << i << "] " << routes[i]->routeName
             << " - Luu luong: " << routes[i]->passengerTraffic << " khach\n";

        StationNode* current = routes[i]->headStation;
        if (!current) cout << "  (Chua co tram nao)\n";

        while (current != nullptr) {
            cout << "  -> Tram " << current->id << ": " << current->name;
            if (current->transferLink != nullptr) {
                cout << " (Trung chuyen sang: " << current->transferLink->id << ")";
            }
            cout << "\n";
            current = current->nextStation;
        }
    }
    cout << "=============================\n";
}

// Hàm dọn dẹp bộ nhớ thủ công (Garbage Collection)
// Đảm bảo không rò rỉ bộ nhớ (Memory Leak)
void garbageCollection() {
    // 1. Xóa tất cả các node trạm từ BST (BST nắm giữ tất cả các trạm)
    deleteBST(bstRoot);
    bstRoot = nullptr;

    // 2. Xóa các đối tượng Route trong mảng con trỏ
    for (int i = 0; i < routeCount; i++) {
        delete routes[i];
        routes[i] = nullptr;
    }
    routeCount = 0;

    cout << "Da don dep toan bo bo nho an toan!\n";
}

int main() {
    // Khởi tạo dữ liệu mẫu
    addRoute("Tuyen 1 (Do)", 15000);
    addRoute("Tuyen 2 (Xanh)", 8000);
    addRoute("Tuyen 3 (Vang)", 20000);

    addStationToRoute(0, "T1_01", "Ga Trung Tam");
    addStationToRoute(0, "T1_02", "Ga Cho Lon");

    addStationToRoute(1, "T2_01", "Ga San Bay");
    addStationToRoute(1, "T2_02", "Ga Ben Xe");

    int choice;
    do {
        cout << "\n=== MO PHONG GIAO THONG CONG CONG ===\n";
        cout << "1. Hien thi cac tuyen duong\n";
        cout << "2. Them tram vao tuyen\n";
        cout << "3. Tao ket noi trung chuyen (Transfer Link)\n";
        cout << "4. Sap xep cac tuyen theo luu luong (QuickSort)\n";
        cout << "5. Kiem tra phan tich thoi gian (String Parsing)\n";
        cout << "0. Don dep bo nho va thoat\n";
        cout << "Lua chon: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                displayRoutes();
                break;
            case 2: {
                int rIdx;
                char id[20], name[50];
                cout << "Chon index tuyen duong: ";
                cin >> rIdx;
                cout << "ID Tram: ";
                cin >> setw(20) >> id;
                cin.ignore();
                cout << "Ten Tram: ";
                cin.getline(name, 50);
                addStationToRoute(rIdx, id, name);
                break;
            }
            case 3: {
                char id1[20], id2[20];
                cout << "ID Tram thu nhat: ";
                cin >> setw(20) >> id1;
                cout << "ID Tram thu hai: ";
                cin >> setw(20) >> id2;
                setTransferLink(id1, id2);
                break;
            }
            case 4:
                cout << "Dang sap xep bang QuickSort...\n";
                quickSortRoutes(routes, 0, routeCount - 1);
                cout << "Hoan tat!\n";
                break;
            case 5: {
                char timeStr[20];
                int h, m;
                cout << "Nhap thoi gian (HH:MM): ";
                cin >> setw(20) >> timeStr;
                if (parseTime(timeStr, &h, &m)) {
                    cout << "Phan tich thanh cong: " << h << " gio " << m << " phut.\n";
                } else {
                    cout << "Loi dinh dang thoi gian!\n";
                }
                break;
            }
            case 0:
                garbageCollection();
                break;
            default:
                cout << "Lua chon khong hop le.\n";
        }
    } while (choice != 0);

    return 0;
}
