#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <limits>

using namespace std;

// ==================== ИСКЛЮЧЕНИЯ ====================
class HotelException : public runtime_error {
public:
    explicit HotelException(const string& message) : runtime_error(message) {}
};

class InvalidPriceException : public HotelException {
public:
    InvalidPriceException() : HotelException("Ошибка: Цена должна быть положительным числом") {}
};
;
class InvalidDiscountException : public HotelException {
public:
    InvalidDiscountException() : HotelException("Ошибка: Скидка должна быть от 0 до 100") {}
};

class RoomNotFoundException : public HotelException {
public:
    RoomNotFoundException() : HotelException("Ошибка: Номер не найден") {}
};

class EmptyHotelException : public HotelException {
public:
    EmptyHotelException() : HotelException("Ошибка: Нет номеров") {}
};

class RoomAlreadyExistsException : public HotelException {
public:
    RoomAlreadyExistsException(const string& roomName)
        : HotelException("Ошибка: Номер \"" + roomName + "\" уже существует") {
    }
};

// ==================== ИНТЕРФЕЙС СТРАТЕГИИ ====================
class IRateStrategy {
public:
    virtual ~IRateStrategy() = default;
    virtual double calculatePrice(double basePrice) const = 0;
};

// ==================== СТРАТЕГИЯ БЕЗ СКИДКИ ====================
class NoDiscountStrategy : public IRateStrategy {
public:
    double calculatePrice(double basePrice) const override {
        return basePrice;
    }
};

// ==================== СТРАТЕГИЯ С ПРОЦЕНТНОЙ СКИДКОЙ ====================
class PercentDiscountStrategy : public IRateStrategy {
private:
    double discountPercent;

public:
    PercentDiscountStrategy(double percent) : discountPercent(percent) {
        if (percent < 0 || percent > 100) {
            throw InvalidDiscountException();
        }
    }

    double calculatePrice(double basePrice) const override {
        return basePrice * (1.0 - discountPercent / 100.0);
    }
};

// ==================== КЛАСС НОМЕР ====================
class Room {
private:
    string name;
    double basePrice;
    shared_ptr<IRateStrategy> strategy;

public:
    Room(const string& name, double basePrice, shared_ptr<IRateStrategy> strategy)
        : name(name), basePrice(basePrice), strategy(strategy) {
        if (basePrice <= 0) {
            throw InvalidPriceException();
        }
    }

    double getFinalPrice() const {
        return strategy->calculatePrice(basePrice);
    }

    string getName() const {
        return name;
    }

    double getBasePrice() const {
        return basePrice;
    }

    void printInfo() const {
        cout << name << " | " << basePrice << " руб. | Итого: " << getFinalPrice() << " руб." << endl;
    }
};

// ==================== КЛАСС ГОСТИНИЦА ====================
class Hotel {
private:
    vector<Room> rooms;

    bool isRoomExists(const string& name) const {
        return any_of(rooms.begin(), rooms.end(),
            [&name](const Room& room) {
                return room.getName() == name;
            });
    }

public:
    void addRoom(const string& name, double price) {
        if (isRoomExists(name)) {
            throw RoomAlreadyExistsException(name);
        }
        rooms.emplace_back(name, price, make_shared<NoDiscountStrategy>());
        cout << "Добавлен номер: " << name << endl;
    }

    void addDiscountedRoom(const string& name, double price, double discountPercent) {
        if (isRoomExists(name)) {
            throw RoomAlreadyExistsException(name);
        }
        rooms.emplace_back(name, price, make_shared<PercentDiscountStrategy>(discountPercent));
        cout << "Добавлен номер со скидкой " << discountPercent << "%: " << name << endl;
    }

    double getAveragePrice() const {
        if (rooms.empty()) {
            throw EmptyHotelException();
        }
        double total = 0;
        for (const auto& room : rooms) {
            total += room.getFinalPrice();
        }
        return total / rooms.size();
    }

    Room findCheapestRoom() const {
        if (rooms.empty()) {
            throw EmptyHotelException();
        }
        auto cheapest = min_element(rooms.begin(), rooms.end(),
            [](const Room& a, const Room& b) {
                return a.getFinalPrice() < b.getFinalPrice();
            });
        return *cheapest;
    }

    Room findMostExpensiveRoom() const {
        if (rooms.empty()) {
            throw EmptyHotelException();
        }
        auto mostExpensive = max_element(rooms.begin(), rooms.end(),
            [](const Room& a, const Room& b) {
                return a.getFinalPrice() < b.getFinalPrice();
            });
        return *mostExpensive;
    }

    void printAllRooms() const {
        if (rooms.empty()) {
            cout << "Нет номеров" << endl;
            return;
        }
        cout << "\n=== Список номеров ===" << endl;
        for (const auto& room : rooms) {
            room.printInfo();
        }
        cout << "=====================\n" << endl;
    }

    bool removeRoom(const string& name) {
        auto it = find_if(rooms.begin(), rooms.end(),
            [&name](const Room& room) {
                return room.getName() == name;
            });
        if (it != rooms.end()) {
            rooms.erase(it);
            cout << "Номер \"" << name << "\" удален" << endl;
            return true;
        }
        cout << "Номер \"" << name << "\" не найден" << endl;
        return false;
    }
};

// ==================== ФУНКЦИЯ ДЛЯ ОЧИСТКИ БУФЕРА ====================
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
int getIntegerInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (cin.fail() or cin.peek() != '\n') {
            cin.clear();
            cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            cout << "Ошибка: Введите нормальное число\n";
        }
        else {
            cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            return value;
        }
    }
}


// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================
int main() {
    setlocale(LC_ALL, "Russian");

    Hotel hotel;
    int choice;
    string name;
    int price, discount;

    cout << "=== Гостиница 'Люкс' ===" << endl;
    cout << "Система управления номерами\n" << endl;

    do {
        cout << "\nМЕНЮ:" << endl;
        cout << "1. Добавить обычный номер" << endl;
        cout << "2. Добавить номер со скидкой" << endl;
        cout << "3. Показать все номера" << endl;
        cout << "4. Средняя стоимость" << endl;
        cout << "5. Самый дешевый номер" << endl;
        cout << "6. Самый дорогой номер" << endl;
        cout << "7. Удалить номер" << endl;
        cout << "0. Выход" << endl;

        choice = getIntegerInput("Выберите действие: ");

        switch (choice) {
        case 1:
            
            cout << "\nНазвание: ";
            getline(cin, name);
            cout << "Цена: ";
            cin >> price;
            while (std::cin.fail() or std::cin.peek() != '\n' or price < 0 or price>1000000) {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                std::cout << "Ошибка: Введите нормальное число\n";
                std::cout << "Цена: ";
                std::cin >> price;
            }
            try {
                hotel.addRoom(name, price);
            }
            catch (const exception& e) {
                cout << e.what() << endl;
            }
            break;

        case 2:
            
            cout << "\nНазвание: ";
            getline(cin, name);
            cout << "Цена: ";
            cin >> price;
            while (std::cin.fail() or std::cin.peek() != '\n' or price < 0 or price>1000000) {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                std::cout << "Ошибка: Введите нормальное число\n";
                std::cout << "Цена: ";
                std::cin >> price;
            }
            cout << "Скидка (%): ";
            cin >> discount;
            while (std::cin.fail() or std::cin.peek() != '\n' or discount < 0) {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                std::cout << "Ошибка: Введите нормальное число\n";
                std::cout << "Скидка (%): ";
                std::cin >> discount;
            }
            try {
                hotel.addDiscountedRoom(name, price, discount);
            }
            catch (const exception& e) {
                cout << e.what() << endl;
            }
            break;

        case 3:
            hotel.printAllRooms();
            break;

        case 4:
            try {
                cout << "Средняя стоимость: " << hotel.getAveragePrice() << " руб." << endl;
            }
            catch (const exception& e) {
                cout << e.what() << endl;
            }
            break;

        case 5:
            try {
                Room cheapest = hotel.findCheapestRoom();
                cout << "Самый дешевый: ";
                cheapest.printInfo();
            }
            catch (const exception& e) {
                cout << e.what() << endl;
            }
            break;

        case 6:
            try {
                Room expensive = hotel.findMostExpensiveRoom();
                cout << "Самый дорогой: ";
                expensive.printInfo();
            }
            catch (const exception& e) {
                cout << e.what() << endl;
            }
            break;

        case 7:
           
            cout << "\nНазвание номера: ";
            getline(cin, name);
            hotel.removeRoom(name);
            break;

        case 0:
            cout << "До свидания!" << endl;
            break;

        default:
            cout << "Неверный выбор (0-7)" << endl;
        }

    } while (choice != 0);

    return 0;
}