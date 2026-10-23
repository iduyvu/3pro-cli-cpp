#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>

using namespace std;

// Cấu trúc Hồ sơ bệnh án (Nút trong danh sách liên kết đôi)
// Nút này tuyệt đối không sao chép dữ liệu khi sắp xếp
struct PatientRecord {
    string id;        // ID 12 chữ số
    string name;      // Tên bệnh nhân
    string dob;       // Ngày sinh DD/MM/YYYY
    int priority;     // Độ ưu tiên (cao hơn = ưu tiên hơn)

    PatientRecord* next; // Con trỏ tới bệnh án kế tiếp
    PatientRecord* prev; // Con trỏ tới bệnh án trước đó

    // Constructor khởi tạo dữ liệu
    PatientRecord(string id, string name, string dob, int priority)
        : id(id), name(name), dob(dob), priority(priority), next(nullptr), prev(nullptr) {}
};

// Cấu trúc Khoa (Quản lý một danh sách liên kết đôi của PatientRecord)
struct Department {
    string name;
    PatientRecord* head; // Con trỏ đầu danh sách
    PatientRecord* tail; // Con trỏ cuối danh sách

    Department(string name) : name(name), head(nullptr), tail(nullptr) {}
};

// Mảng con trỏ quản lý tối đa 20 khoa
Department* depts[20];
int deptCount = 0;

// Hàm chuẩn hóa chuỗi bằng con trỏ (xóa khoảng trắng thừa, viết hoa chữ cái đầu)
void standardizeName(string& s) {
    if (s.empty()) return;

    // Tạo buffer cục bộ để thao tác hoàn toàn bằng con trỏ char*
    char* buffer = new char[s.length() + 1];
    strcpy(buffer, s.c_str());

    char* src = buffer;
    char* dest = buffer;
    bool inSpace = true; // Bắt đầu bằng space để viết hoa chữ đầu tiên

    while (*src != '\0') {
        if (*src == ' ') {
            if (!inSpace) {
                *dest = ' ';
                dest++;
                inSpace = true;
            }
        } else {
            if (inSpace) {
                // Viết hoa chữ cái đầu của từ bằng phép tính ASCII
                if (*src >= 'a' && *src <= 'z') {
                    *dest = *src - 32;
                } else {
                    *dest = *src;
                }
                inSpace = false;
            } else {
                // Viết thường các chữ cái còn lại
                if (*src >= 'A' && *src <= 'Z') {
                    *dest = *src + 32;
                } else {
                    *dest = *src;
                }
            }
            dest++;
        }
        src++;
    }

    // Xóa khoảng trắng cuối cùng nếu có
    if (dest > buffer && *(dest - 1) == ' ') {
        dest--;
    }
    *dest = '\0'; // Đánh dấu kết thúc chuỗi

    s = string(buffer);
    delete[] buffer; // Giải phóng bộ nhớ động của buffer
}

// Kiểm tra định dạng ngày sinh (DD/MM/YYYY) bằng các phép toán con trỏ
bool validateDoB(const string& dob) {
    if (dob.length() != 10) return false;
    const char* ptr = dob.c_str();

    // DD: Phải từ 00 đến 39 (chấp nhận cơ bản)
    if (!(*ptr >= '0' && *ptr <= '3') || !(*(ptr+1) >= '0' && *(ptr+1) <= '9')) return false;
    if (*(ptr+2) != '/') return false;

    // MM: Phải từ 00 đến 19
    ptr += 3;
    if (!(*ptr >= '0' && *ptr <= '1') || !(*(ptr+1) >= '0' && *(ptr+1) <= '9')) return false;
    if (*(ptr+2) != '/') return false;

    // YYYY: Phải là số
    ptr += 3;
    for (int i = 0; i < 4; ++i) {
        if (!(*(ptr+i) >= '0' && *(ptr+i) <= '9')) return false;
    }

    return true;
}

// Kiểm tra ID (12 chữ số) bằng con trỏ
bool validateID(const string& id) {
    if (id.length() != 12) return false;
    const char* ptr = id.c_str();
    while (*ptr != '\0') {
        if (*ptr < '0' || *ptr > '9') return false; // Chỉ cho phép ký tự số
        ptr++;
    }
    return true;
}

// Thêm một khoa mới vào mảng con trỏ
void addDepartment(const string& name) {
    if (deptCount < 20) {
        depts[deptCount] = new Department(name);
        deptCount++;
        cout << "Da them khoa: " << name << endl;
    } else {
        cout << "Khong the them khoa, danh sach da day!\n";
    }
}

// Thêm bệnh nhân vào cuối danh sách của một khoa
void addPatient(int deptIndex, string id, string name, string dob, int priority) {
    if (deptIndex < 0 || deptIndex >= deptCount) {
        cout << "Khoa khong hop le.\n";
        return;
    }

    standardizeName(name);

    if (!validateID(id)) {
        cout << "Loi: ID phai bao gom 12 chu so.\n";
        return;
    }

    if (!validateDoB(dob)) {
        cout << "Loi: Ngay sinh phai co dinh dang DD/MM/YYYY.\n";
        return;
    }

    PatientRecord* newPatient = new PatientRecord(id, name, dob, priority);
    Department* dept = depts[deptIndex];

    // Thêm vào cuối danh sách liên kết đôi
    if (dept->tail == nullptr) {
        dept->head = dept->tail = newPatient;
    } else {
        dept->tail->next = newPatient;
        newPatient->prev = dept->tail;
        dept->tail = newPatient;
    }
    cout << "Da them benh nhan thanh cong!\n";
}

// Hàm chuyển bệnh nhân từ khoa này sang khoa khác
// Luật cốt lõi: Cắt node từ danh sách này và nối vào danh sách khác bằng cách thao tác 4 con trỏ cùng lúc
bool transferDepartment(int fromIndex, int toIndex, const string& patientId) {
    if (fromIndex < 0 || fromIndex >= deptCount || toIndex < 0 || toIndex >= deptCount) return false;

    Department* fromDept = depts[fromIndex];
    Department* toDept = depts[toIndex];

    // 1. Tìm bệnh nhân trong khoa hiện tại
    PatientRecord* current = fromDept->head;
    while (current != nullptr && current->id != patientId) {
        current = current->next;
    }

    if (current == nullptr) {
        cout << "Khong tim thay benh nhan trong khoa goc.\n";
        return false; // Không tìm thấy
    }

    // 2. Cắt node khỏi danh sách fromDept
    if (current->prev != nullptr) {
        current->prev->next = current->next; // Con trỏ 1: Nối next của node trước qua node hiện tại
    } else {
        fromDept->head = current->next; // Cập nhật head nếu cắt ở đầu
    }

    if (current->next != nullptr) {
        current->next->prev = current->prev; // Con trỏ 2: Nối prev của node sau qua node hiện tại
    } else {
        fromDept->tail = current->prev; // Cập nhật tail nếu cắt ở cuối
    }

    // 3. Nối node vào cuối danh sách toDept
    current->next = nullptr;
    current->prev = toDept->tail; // Con trỏ 3: Trỏ prev của node về tail của khoa mới

    if (toDept->tail != nullptr) {
        toDept->tail->next = current; // Con trỏ 4: Trỏ next của tail cũ về node mới
    } else {
        toDept->head = current; // Nếu danh sách toDept đang trống
    }
    toDept->tail = current; // Cập nhật tail của khoa mới

    cout << "Da chuyen benh nhan thanh cong!\n";
    return true;
}

// ==================== HỆ THỐNG MERGE SORT ====================
// Chia danh sách thành 2 nửa
PatientRecord* splitList(PatientRecord* head) {
    PatientRecord* fast = head;
    PatientRecord* slow = head;

    // Slow đi 1 bước, fast đi 2 bước để tìm điểm giữa
    while (fast->next != nullptr && fast->next->next != nullptr) {
        fast = fast->next->next;
        slow = slow->next;
    }

    PatientRecord* temp = slow->next;
    slow->next = nullptr; // Cắt đôi danh sách
    return temp;
}

// Trộn 2 danh sách đã sắp xếp (Chỉ thao tác thay đổi con trỏ, tuyệt đối không copy dữ liệu)
PatientRecord* mergeLists(PatientRecord* first, PatientRecord* second) {
    if (!first) return second;
    if (!second) return first;

    // Sắp xếp theo priority giảm dần
    if (first->priority > second->priority) {
        first->next = mergeLists(first->next, second);
        first->next->prev = first; // Liên kết ngược
        first->prev = nullptr;
        return first;
    } else {
        second->next = mergeLists(first, second->next);
        second->next->prev = second; // Liên kết ngược
        second->prev = nullptr;
        return second;
    }
}

// Hàm đệ quy Merge Sort trên danh sách liên kết đôi
PatientRecord* mergeSort(PatientRecord* head) {
    if (!head || !head->next) return head; // Base case: 0 hoặc 1 node

    PatientRecord* secondHalf = splitList(head);

    // Sắp xếp nửa đầu và nửa sau
    head = mergeSort(head);
    secondHalf = mergeSort(secondHalf);

    // Trộn hai nửa
    return mergeLists(head, secondHalf);
}

// Giao diện sắp xếp cho Khoa
void sortDepartmentPatients(int deptIndex) {
    if (deptIndex < 0 || deptIndex >= deptCount) return;
    Department* dept = depts[deptIndex];
    if (!dept || !dept->head) return;

    // Thực hiện Merge Sort
    dept->head = mergeSort(dept->head);

    // Quét lại danh sách để cập nhật con trỏ tail
    PatientRecord* temp = dept->head;
    while (temp != nullptr && temp->next != nullptr) {
        temp = temp->next;
    }
    dept->tail = temp;
    cout << "Da sap xep danh sach benh nhan thanh cong (Bang Merge Sort).\n";
}
// =============================================================

// Hiển thị danh sách bệnh nhân
void displayDepartment(int deptIndex) {
    if (deptIndex < 0 || deptIndex >= deptCount) return;
    Department* dept = depts[deptIndex];

    cout << "\n=== Khoa: " << dept->name << " ===\n";
    cout << left << setw(15) << "ID" << setw(25) << "Ten" << setw(15) << "Ngay Sinh" << "Uu Tien\n";
    cout << "------------------------------------------------------------\n";

    PatientRecord* current = dept->head;
    while (current != nullptr) {
        cout << left << setw(15) << current->id
             << setw(25) << current->name
             << setw(15) << current->dob
             << current->priority << "\n";
        current = current->next;
    }
    cout << "------------------------------------------------------------\n";
}

// Xóa toàn bộ bộ nhớ để chống rò rỉ (Memory leak)
void freeAllMemory() {
    for (int i = 0; i < deptCount; i++) {
        PatientRecord* current = depts[i]->head;
        while (current != nullptr) {
            PatientRecord* nextNode = current->next;
            delete current; // Giải phóng node bệnh án
            current = nextNode;
        }
        delete depts[i]; // Giải phóng khoa
    }
}

int main() {
    // Khởi tạo một số khoa cơ bản
    addDepartment("Khoa Cap Cuu");
    addDepartment("Khoa Noi");
    addDepartment("Khoa Ngoai");

    int choice;
    do {
        cout << "\n=== HE THONG QUAN LY HO SO BENH AN ===\n";
        cout << "1. Them benh nhan\n";
        cout << "2. Hien thi danh sach benh nhan theo khoa\n";
        cout << "3. Sap xep benh nhan theo do uu tien (Merge Sort)\n";
        cout << "4. Chuyen khoa cho benh nhan\n";
        cout << "0. Thoat\n";
        cout << "Lua chon cua ban: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                int dIdx;
                string id, name, dob;
                int prio;
                cout << "Chon khoa (0: Cap Cuu, 1: Noi, 2: Ngoai): ";
                cin >> dIdx;
                cout << "ID (12 so): ";
                cin >> id;
                cin.ignore();
                cout << "Ho ten (chua chuan hoa): ";
                getline(cin, name);
                cout << "Ngay sinh (DD/MM/YYYY): ";
                cin >> dob;
                cout << "Do uu tien (So cang lon cang uu tien): ";
                cin >> prio;
                addPatient(dIdx, id, name, dob, prio);
                break;
            }
            case 2: {
                int dIdx;
                cout << "Chon khoa (0: Cap Cuu, 1: Noi, 2: Ngoai): ";
                cin >> dIdx;
                displayDepartment(dIdx);
                break;
            }
            case 3: {
                int dIdx;
                cout << "Chon khoa (0: Cap Cuu, 1: Noi, 2: Ngoai): ";
                cin >> dIdx;
                sortDepartmentPatients(dIdx);
                break;
            }
            case 4: {
                int from, to;
                string id;
                cout << "Khoa hien tai (0-2): ";
                cin >> from;
                cout << "Khoa muon chuyen den (0-2): ";
                cin >> to;
                cout << "ID benh nhan can chuyen: ";
                cin >> id;
                transferDepartment(from, to, id);
                break;
            }
            case 0:
                cout << "Giai phong bo nho va thoat...\n";
                freeAllMemory();
                break;
            default:
                cout << "Lua chon khong hop le.\n";
        }
    } while (choice != 0);

    return 0;
}
